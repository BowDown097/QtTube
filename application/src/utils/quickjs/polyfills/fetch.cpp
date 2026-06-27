#include "fetch.h"
#include "utils/quickjs/qjsutils.h"
#include "utils/quickjs/qt_js_traits.h"
#include <qttube-plugin/utils/httprequest.h>
#include <quickjs++/context.h>

namespace jsfetch
{
    struct RequestInit
    {
        QByteArray method = "GET";
        QList<std::pair<QByteArray, QByteArray>> headers;
        QByteArray body;

        RequestInit() = default;
        explicit RequestInit(const qjs::value& options)
            : method(QJSUtils::getStringStrict<QByteArray, true>(options["method"], "", "GET")),
              headers(QJSUtils::unwrapObjectProperty<QList<std::pair<QByteArray, QByteArray>>>(options.ctx, options.v, "headers")),
              body(QJSUtils::unwrapObjectProperty<QByteArray>(options.ctx, options.v, "body")) {}
    };

    Response::Response(JSContext* ctx, const qjs::rest<qjs::value>& args)
        : headers(ctx, JS_NewObject(ctx)),
          m_body(std::make_unique<MemoryBody>(!args.empty() ? args[0].as<QByteArray>() : QByteArray())),
          m_ctx(ctx)
    {
        if (args.size() < 2 || !JS_IsObject(args[1].v))
            return;

        if (qjs::value statusVal = args[1]["status"]; JS_IsNumber(statusVal.v))
            status = statusVal.as<int>();
        if (qjs::value statusTextVal = args[1]["statusText"]; JS_IsString(statusTextVal.v))
            statusText = statusTextVal.as<QByteArray>();
        if (qjs::value headersVal = args[1]["headers"]; JS_IsObject(headersVal.v))
            headers = headersVal;
    }

    Response::Response(JSContext* ctx, QNetworkReply* reply, int status)
        : headers(ctx, JS_NewObject(ctx)),
          ok(status >= 200 && status < 300),
          redirected(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isValid()),
          status(status),
          statusText(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).value<QByteArray>()),
          url(reply->url().toString(QUrl::PrettyDecoded | QUrl::RemoveFragment).toUtf8()),
          m_body(std::make_unique<ReplyBody>(reply)),
          m_ctx(ctx) {}

    JSValue Response::arrayBuffer()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx, "Body has already been consumed.");
        bodyUsed = true;

        const QByteArray data = m_body->readAll();
        return JS_NewArrayBufferCopy(m_ctx, reinterpret_cast<const uint8_t*>(data.data()), data.size());
    }

    JSValue Response::json()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx, "Body has already been consumed.");
        bodyUsed = true;

        const QByteArray data = m_body->readAll();
        return JS_ParseJSON(m_ctx, data.data(), data.size(), "<dump>");
    }

    JSValue Response::text()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx, "Body has already been consumed.");
        bodyUsed = true;
        return qjs::js_traits<QByteArray>::wrap(m_ctx, m_body->readAll());
    }

    void fetchRequest(
        const QUrl& url, const RequestInit& options,
        JSContext* ctx, JSValue resolve, JSValue reject)
    {
        QNetworkRequest req(url);
        req.setAttribute(QNetworkRequest::AutoDeleteReplyOnFinishAttribute, false);
        for (const auto& [key, value] : options.headers)
            req.setRawHeader(key, value);

        QNetworkReply* reply;
        if (options.method.compare("GET", Qt::CaseInsensitive) == 0)
            reply = HttpReply::networkAccessManager()->get(req);
        else if (options.method.compare("POST", Qt::CaseInsensitive) == 0)
            reply = HttpReply::networkAccessManager()->post(req, options.body);
        else if (options.method.compare("HEAD", Qt::CaseInsensitive) == 0)
            reply = HttpReply::networkAccessManager()->head(req);
        else if (options.method.compare("DELETE", Qt::CaseInsensitive) == 0)
            reply = HttpReply::networkAccessManager()->deleteResource(req);
        else if (options.method.compare("PUT", Qt::CaseInsensitive) == 0)
            reply = HttpReply::networkAccessManager()->put(req, options.body);
        else
            reply = HttpReply::networkAccessManager()->sendCustomRequest(req, options.method, options.body);

        auto throwError = [=](auto f, const char* fmt, auto&&... args) {
            JSValue err = f(ctx, fmt, std::forward<decltype(args)>(args)...);
            JS_Call(ctx, reject, JS_UNDEFINED, 1, &err);

            JS_FreeValue(ctx, err);
            JS_FreeValue(ctx, resolve);
            JS_FreeValue(ctx, reject);

            reply->deleteLater();
        };

        QObject::connect(reply, &QNetworkReply::finished, [=] {
            if (reply->error() != QNetworkReply::NoError)
            {
                throwError(
                    JS_ThrowTypeError,
                    "NetworkError when attempting to fetch resource: %s",
                    reply->errorString().toUtf8().constData());
                return;
            }

            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (status < 200 || status > 599)
            {
                throwError(JS_ThrowRangeError, "Invalid status code.");
                return;
            }

            std::shared_ptr<Response> response = std::make_shared<Response>(ctx, reply, status);
            for (const auto& [key, value] : reply->rawHeaderPairs())
                response->headers[key.constData()] = value;

            qjs::value responseValue(ctx, std::move(response));
            JS_Call(ctx, resolve, JS_UNDEFINED, 1, &responseValue.v);

            JS_FreeValue(ctx, resolve);
            JS_FreeValue(ctx, reject);
        });
    }

    JSValue fetch(JSContext* ctx, const qjs::rest<qjs::value>& args)
    {
        if (args.empty())
            return JS_ThrowTypeError(ctx, "fetch: At least 1 argument required, but only 0 passed");

        QUrl url(args[0].as<QByteArray>());
        if (url.scheme() != "http" && url.scheme() != "https")
            return JS_ThrowTypeError(ctx, "fetch: Scheme must be HTTP(S)");

        RequestInit options = args.size() > 1 ? RequestInit(args[1]) : RequestInit();
        JSValue resolving_funcs[2];
        JSValue promise = JS_NewPromiseCapability(ctx, resolving_funcs);
        fetchRequest(url, options, ctx, resolving_funcs[0], resolving_funcs[1]);
        return promise;
    }

    void registerFor(qjs::context& ctx, qjs::module* mod)
    {
        qjs::class_registrar<Response>(ctx, mod)
            .constructor<JSContext*, const qjs::rest<qjs::value>&>()
            .mark<&Response::headers>()
            .member<&Response::bodyUsed>()
            .member<&Response::headers>()
            .member<&Response::ok>()
            .member<&Response::redirected>()
            .member<&Response::status>()
            .member<&Response::statusText>()
            .member<&Response::url>()
            .member<&Response::arrayBuffer>()
            .member<&Response::json>()
            .member<&Response::text>();

        if (mod)
            mod->add("fetch", &fetch);
        else
            ctx.global()["fetch"] = fetch;
    }
}