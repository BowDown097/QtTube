#include "fetch.h"
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

        static QByteArray getBody(const qjs::value& val)
        {
            return val["body"].as<QByteArray>();
        }

        static QList<std::pair<QByteArray, QByteArray>> getHeaders(const qjs::value& val)
        {
            return val["headers"].as<QList<std::pair<QByteArray, QByteArray>>>();
        }

        static QByteArray getMethod(const qjs::value& val)
        {
            qjs::value methodVal = val["method"];
            return JS_IsString(methodVal.v) ? methodVal.as<QByteArray>() : QByteArray("GET");
        }
    };

    Response::Response(qjs::context& ctx, QNetworkReply* reply, int status)
        : headers(ctx.new_object()),
          ok(status >= 200 && status < 300),
          redirected(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isValid()),
          status(status),
          statusText(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).value<QByteArray>()),
          url(reply->url().toString(QUrl::PrettyDecoded | QUrl::RemoveFragment).toUtf8()),
          m_ctx(ctx),
          m_reply(reply) {}

    Response::~Response()
    {
        m_reply->deleteLater();
    }

    JSValue Response::arrayBuffer()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx.ctx, "Body has already been consumed.");
        bodyUsed = true;

        const QByteArray data = m_reply->readAll();
        return JS_NewArrayBufferCopy(m_ctx.ctx, reinterpret_cast<const uint8_t*>(data.data()), data.size());
    }

    JSValue Response::json()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx.ctx, "Body has already been consumed.");
        bodyUsed = true;

        const QByteArray data = m_reply->readAll();
        return JS_ParseJSON(m_ctx.ctx, data.data(), data.size(), "<dump>");
    }

    JSValue Response::text()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx.ctx, "Body has already been consumed.");
        bodyUsed = true;
        return qjs::js_traits<QByteArray>::wrap(m_ctx.ctx, m_reply->readAll());
    }

    void fetchRequest(
        const QUrl& url, const RequestInit& options,
        qjs::context& ctx, JSValue resolve, JSValue reject)
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

        auto throwError = [=, &ctx](auto f, const char* fmt, auto&&... args) {
            JSValue err = f(ctx.ctx, fmt, std::forward<decltype(args)>(args)...);
            JS_Call(ctx.ctx, reject, JS_UNDEFINED, 1, &err);

            JS_FreeValue(ctx.ctx, err);
            JS_FreeValue(ctx.ctx, resolve);
            JS_FreeValue(ctx.ctx, reject);

            reply->deleteLater();
        };

        QObject::connect(reply, &QNetworkReply::finished, [=, &ctx] {
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

            qjs::value responseValue = ctx.new_value(std::move(response));
            JS_Call(ctx.ctx, resolve, JS_UNDEFINED, 1, &responseValue.v);

            JS_FreeValue(ctx.ctx, resolve);
            JS_FreeValue(ctx.ctx, reject);
        });
    }

    JSValue fetch(qjs::context& ctx, const qjs::rest<qjs::value>& args)
    {
        if (args.empty())
            return JS_ThrowTypeError(ctx.ctx, "fetch: At least 1 argument required, but only 0 passed");

        QUrl url(args[0].as<QByteArray>());
        if (url.scheme() != "http" && url.scheme() != "https")
            return JS_ThrowTypeError(ctx.ctx, "fetch: Scheme must be HTTP(S)");

        RequestInit options;
        if (args.size() > 1)
        {
            if (!JS_IsObject(args[1].v))
                return JS_ThrowTypeError(ctx.ctx, "fetch: Argument 2 can't be converted to a dictionary.");
            options.body = RequestInit::getBody(args[1]);
            options.headers = RequestInit::getHeaders(args[1]);
            options.method = RequestInit::getMethod(args[1]);
        }

        JSValue resolving_funcs[2];
        JSValue promise = JS_NewPromiseCapability(ctx.ctx, resolving_funcs);
        fetchRequest(url, options, ctx, resolving_funcs[0], resolving_funcs[1]);
        return promise;
    }

    void registerFor(qjs::context& ctx, qjs::module* mod)
    {
        qjs::class_registrar<Response>("Response", ctx, mod)
            .mark<&Response::headers>()
            .member<&Response::bodyUsed>("bodyUsed")
            .member<&Response::headers>("headers")
            .member<&Response::ok>("ok")
            .member<&Response::redirected>("redirected")
            .member<&Response::status>("status")
            .member<&Response::statusText>("statusText")
            .member<&Response::url>("url")
            .member<&Response::arrayBuffer>("arrayBuffer")
            .member<&Response::json>("json")
            .member<&Response::text>("text");

        if (mod)
            mod->add("fetch", [&](const qjs::rest<qjs::value>& args) { return fetch(ctx, args); });
        else
            ctx.global()["fetch"] = [&](const qjs::rest<qjs::value>& args) { return fetch(ctx, args); };
    }
}