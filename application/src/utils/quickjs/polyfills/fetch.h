#pragma once
#include <QByteArray>
#include <QNetworkReply>
#include <QPointer>
#include <quickjs++/value.h>

namespace jsfetch
{
    struct ResponseBody
    {
        virtual QByteArray readAll() = 0;
        virtual ~ResponseBody() = default;
    };

    struct MemoryBody : ResponseBody
    {
        QByteArray data;
        explicit MemoryBody(const QByteArray& data) : data(data) {}
        QByteArray readAll() override { return std::move(data); }
    };

    struct ReplyBody : ResponseBody
    {
        QPointer<QNetworkReply> reply;
        explicit ReplyBody(QNetworkReply* reply) : reply(reply) {}
        QByteArray readAll() override { return reply->readAll(); }
        ~ReplyBody() override { reply->deleteLater(); }
    };

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

        Response(JSContext* ctx, const qjs::rest<qjs::value>& args);
        Response(JSContext* ctx, QNetworkReply* reply, int status);

        JSValue arrayBuffer();
        JSValue json();
        JSValue text();
    private:
        std::unique_ptr<ResponseBody> m_body;
        JSContext* m_ctx;
    };

    JSValue fetch(JSContext* ctx, const qjs::rest<qjs::value>& args);
    void registerFor(qjs::context& ctx, qjs::module* mod = nullptr);
}