#ifndef PLAYERINTERCEPTOR_H
#define PLAYERINTERCEPTOR_H
#include "http.h"
#include "innertube/itc-objects/innertubeauthstore.h"
#include "innertube/responses/video/playerresponse.h"
#include <QWebEngineUrlRequestInterceptor>

class PlayerInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit PlayerInterceptor(QObject* parent = nullptr) : QWebEngineUrlRequestInterceptor(parent) {}
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;

    void setAuthStore(InnertubeAuthStore* authStore) { this->m_authStore = authStore; }
    void setContext(InnertubeContext* context) { this->m_context = context; }
    void setPlayerResponse(const InnertubeEndpoints::PlayerResponse& resp) { this->m_playerResponse = resp; }
    void setUsePlaybackTracking(bool playbackTracking) { this->m_playbackTracking = playbackTracking; }
    void setUseWatchtimeTracking(bool watchtimeTracking) { this->m_watchtimeTracking = watchtimeTracking; }
private:
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
    InnertubeAuthStore* m_authStore = nullptr;
    InnertubeContext* m_context = nullptr;
    InnertubeEndpoints::PlayerResponse m_playerResponse;
    bool m_playbackTracking = false;
    bool m_watchtimeTracking = false;
};

#endif // PLAYERINTERCEPTOR_H
