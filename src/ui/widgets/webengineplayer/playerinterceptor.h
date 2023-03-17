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
private:
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
    QPointer<InnertubeAuthStore> m_authStore;
    InnertubeContext* m_context;
    InnertubeEndpoints::PlayerResponse m_playerResponse;
};

#endif // PLAYERINTERCEPTOR_H
