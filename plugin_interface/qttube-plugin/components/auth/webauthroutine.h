#pragma once
#include "authroutine.h"
#include <QHash>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QWebEngineUrlRequestInterceptor>
#endif

class QNetworkCookie;

namespace QtTubePlugin
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    class WebAuthRequestInterceptor : public QWebEngineUrlRequestInterceptor
    {
        Q_OBJECT
    public:
        WebAuthRequestInterceptor(const QList<QByteArray>& searchHeaders, QObject* parent = nullptr)
            : QWebEngineUrlRequestInterceptor(parent), m_searchHeaders(searchHeaders) {}
        void interceptRequest(QWebEngineUrlRequestInfo& info) override;
    private:
        QList<QByteArray> m_searchHeaders;
    signals:
        void foundHeader(const QByteArray& key, const QByteArray& value);
    };
#endif

    struct SearchCookie
    {
        QByteArray name;
        QString domain;
        QString path;
    };

    class WebAuthRoutine : public AuthRoutine
    {
        Q_OBJECT
    public:
        QHash<QByteArray, QByteArray> searchCookies() const;
        void setSearchCookies(const QList<SearchCookie>& cookies);
        virtual void onNewCookie(const QByteArray& name, const QByteArray& value) {}

        void setUrl(const QUrl& url) { m_url = url; }
        void start() override;
    protected:
        QList<std::pair<SearchCookie, QByteArray>> m_searchCookies;
        QUrl m_url;
    private:
        bool nothingToSearch() const;
    private slots:
        void cookieAdded(const QNetworkCookie& cookie);

    // Qt does not provide any way to intercept headers until Qt 6.5
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    public:
        const QHash<QByteArray, QByteArray>& searchHeaders() const { return m_searchHeaders; }
        void setSearchHeaders(const QList<QByteArray>& headers);
        virtual void onNewHeader(const QByteArray& name, const QByteArray& value) {}
    protected:
        QHash<QByteArray, QByteArray> m_searchHeaders;
    private:
        WebAuthRequestInterceptor* m_interceptor;
    private slots:
        void foundHeader(const QByteArray& name, const QByteArray& value);
    #endif
    };
}
