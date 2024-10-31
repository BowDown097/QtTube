#pragma once
#include "innertube/itc-objects/innertubeauthstore.h"
#include "innertube/responses/video/playerresponse.h"
#include <QPointer>
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
    void setPlayerResponse(const InnertubeEndpoints::PlayerResponse& resp) { this->m_playerResponse = resp; }
private:
    QPointer<InnertubeAuthStore> m_authStore;
    InnertubeContext* m_context{};
    InnertubeEndpoints::PlayerResponse m_playerResponse;

    void doWatchtime(QWebEngineUrlRequestInfo& info);
    void doPlayback(QWebEngineUrlRequestInfo& info);
    bool isTrackingUrl(const QUrl& url);
};
