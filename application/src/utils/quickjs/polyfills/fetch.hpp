#pragma once
#include <qttube-plugin/utils/httprequest.h>
#include <quickjs++/value.h>

namespace jsfetch
{
    class Response
    {
    public:
        bool bodyUsed = false;
        HttpReply::HeaderList headers;
        bool ok = true;
        bool redirected = false;
        int status = 200;
        QByteArray statusText;
        QByteArray url;

        Response(JSContext* ctx, const qjs::rest<qjs::value>& args);
        Response(JSContext* ctx, const HttpReply& reply, int status);

        JSValue arrayBuffer();
        JSValue json();
        JSValue text();
    private:
        QByteArray m_body;
        JSContext* m_ctx;
    };

    JSValue fetch(JSContext* ctx, const qjs::rest<qjs::value>& args);
    void registerFor(qjs::context& ctx, qjs::module* mod = nullptr);
}