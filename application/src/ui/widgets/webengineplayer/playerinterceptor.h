#pragma once
#include "innertube/itc-objects/innertubeauthstore.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
#include <QWebEngineUrlRequestInterceptor>
#endif

class PlayerInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit PlayerInterceptor(QObject* parent = nullptr) : QWebEngineUrlRequestInterceptor(parent) {}
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
    void setAuthStore(InnertubeAuthStore* authStore) { this->m_authStore = authStore; }
    void setContext(InnertubeContext* context) { this->m_context = context; }
private:
    InnertubeAuthStore* m_authStore{};
    InnertubeContext* m_context{};

    bool isTrackingUrl(const QUrl& url);
};
