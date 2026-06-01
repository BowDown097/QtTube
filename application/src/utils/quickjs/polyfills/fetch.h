#pragma once
#include <QByteArray>
#include <quickjs++/value.h>

class QNetworkReply;

namespace jsfetch
{
    class Response
    {
    public:
        bool bodyUsed = false;
        qjs::value headers;
        bool ok = true;
        bool redirected = false;
        int status = 200;
        QByteArray statusText;
        QByteArray url;

        Response(qjs::context& ctx, QNetworkReply* reply, int status);
        ~Response();

        JSValue arrayBuffer();
        JSValue json();
        JSValue text();
    private:
        qjs::context& m_ctx;
        QNetworkReply* m_reply;
    };

    JSValue fetch(qjs::context& ctx, const qjs::rest<qjs::value>& args);
    void registerFor(qjs::context& ctx, qjs::module* mod = nullptr);
}