#ifndef PLAYERINTERCEPTOR_H
#define PLAYERINTERCEPTOR_H
#include "http.h"
#include "innertube/endpoints/video/player.h"
#include "innertube/itc-objects/innertubeauthstore.h"
#include <QWebEngineUrlRequestInterceptor>

class PlayerInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    PlayerInterceptor(InnertubeContext* context, InnertubeAuthStore* authStore, const InnertubeEndpoints::Player& player, bool playbackTracking,
                      bool watchtimeTracking, QObject* p = nullptr);
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
private:
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
    InnertubeAuthStore* authStore;
    InnertubeContext* context;
    InnertubeEndpoints::Player player;
    bool playbackTracking;
    bool watchtimeTracking;
};

#endif // PLAYERINTERCEPTOR_H
