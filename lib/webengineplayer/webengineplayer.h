#ifndef WEBENGINEPLAYER_H
#define WEBENGINEPLAYER_H
#include "fullscreenwindow.h"
#include "playerinterceptor.h"
#include <QWebEngineFullScreenRequest>

class WebEnginePlayer : public QWidget
{
    Q_OBJECT
public:
    explicit WebEnginePlayer(QWidget* parent = nullptr);

    void setAuthStore(InnertubeAuthStore* authStore) { m_interceptor->setAuthStore(authStore); }
    void setContext(InnertubeContext* context) { m_interceptor->setContext(context); }
    void setPlayerResponse(const InnertubeEndpoints::PlayerResponse& resp) { m_interceptor->setPlayerResponse(resp); }
public slots:
    void play(const QString& vId, int progress);
    void seek(int progress);
private slots:
    void fullScreenRequested(QWebEngineFullScreenRequest request);
private:
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;
    PlayerInterceptor* m_interceptor;
    QWebEngineView* m_view = nullptr;
};

#endif // WEBENGINEPLAYER_H
