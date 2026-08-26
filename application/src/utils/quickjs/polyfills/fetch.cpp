#include "fetch.hpp"
#include "utils/quickjs/qjsutils.hpp"
#include <quickjs++/context.h>

namespace jsfetch
{
    struct RequestInit
    {
        QByteArray method = "GET";
        HttpReply::HeaderList headers;
        QByteArray body;
        bool spoofUserAgent{};

        RequestInit() = default;
        explicit RequestInit(const qjs::value& options)
            : method(QJSUtils::getStringStrict<QByteArray, true>(options["method"], "", "GET")),
              headers(QJSUtils::unwrapObjectProperty<HttpReply::HeaderList>(options.ctx, options.v, "headers")),
              body(QJSUtils::unwrapObjectProperty<QByteArray>(options.ctx, options.v, "body")),
              spoofUserAgent(options["spoofUserAgent"].as<bool>()) {}
    };

    Response::Response(JSContext* ctx, const qjs::rest<qjs::value>& args)
        : m_body(!args.empty() ? args[0].as<QByteArray>() : QByteArray()),
          m_ctx(ctx)
    {
        if (args.size() < 2 || !args[1].is_object())
            return;

        if (qjs::value statusVal = args[1]["status"]; statusVal.is_number())
            status = statusVal.as<int>();
        if (qjs::value statusTextVal = args[1]["statusText"]; statusTextVal.is_string())
            statusText = statusTextVal.as<QByteArray>();
        if (qjs::value headersVal = args[1]["headers"]; headersVal.is_object())
            headers = headersVal.as<HttpReply::HeaderList>();
    }

    Response::Response(JSContext* ctx, const HttpReply& reply, int status)
        : headers(reply.headers()),
          ok(status >= 200 && status < 300),
          redirected(reply.attribute(QNetworkRequest::RedirectionTargetAttribute).isValid()),
          status(status),
          statusText(reply.attribute(QNetworkRequest::HttpReasonPhraseAttribute).value<QByteArray>()),
          url(reply.url().toString(QUrl::PrettyDecoded | QUrl::RemoveFragment).toUtf8()),
          m_body(reply.readAll()),
          m_ctx(ctx) {}

    JSValue Response::arrayBuffer()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx, "Body has already been consumed.");
        bodyUsed = true;

        return JS_NewArrayBufferCopy(m_ctx, reinterpret_cast<const uint8_t*>(m_body.data()), m_body.size());
    }

    JSValue Response::json()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx, "Body has already been consumed.");
        bodyUsed = true;

        return JS_ParseJSON(m_ctx, m_body.data(), m_body.size(), "<dump>");
    }

    JSValue Response::text()
    {
        if (bodyUsed)
            return JS_ThrowTypeError(m_ctx, "Body has already been consumed.");
        bodyUsed = true;
        return qjs::js_traits<QByteArray>::wrap(m_ctx, m_body);
    }

    void fetchRequest(
        const QUrl& url, const RequestInit& options,
        JSContext* ctx, JSValue resolve, JSValue reject)
    {
        const HttpReply* reply = HttpRequest()
            .withHeaders(options.headers)
            .withUserAgentSpoofing(options.spoofUserAgent)
            .request(url, options.method, options.body);

        auto throwError = [=](auto f, const char* fmt, auto&&... args) {
            JSValue err = f(ctx, fmt, std::forward<decltype(args)>(args)...);
            JS_Call(ctx, reject, JS_UNDEFINED, 1, &err);

            JS_FreeValue(ctx, err);
            JS_FreeValue(ctx, resolve);
            JS_FreeValue(ctx, reject);
        };

        QObject::connect(reply, &HttpReply::finished, [=](const HttpReply& reply) {
            if (reply.error() != QNetworkReply::NoError)
            {
                throwError(
                    JS_ThrowTypeError,
                    "NetworkError when attempting to fetch resource: %s",
                    reply.errorString().toUtf8().constData());
                return;
            }

            int status = reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (status < 200 || status > 599)
            {
                throwError(JS_ThrowRangeError, "Invalid status code.");
                return;
            }

            qjs::value responseValue(ctx, std::make_shared<Response>(ctx, reply, status));
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