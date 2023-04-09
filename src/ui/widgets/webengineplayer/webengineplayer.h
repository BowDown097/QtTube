#ifndef WEBENGINEPLAYER_H
#define WEBENGINEPLAYER_H
#include "fullscreenwindow.h"
#include "playerinterceptor.h"
#include "webchannelinterface.h"
#include <QWebEngineFullScreenRequest>
#include <QWebEngineScript>

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
    WebChannelInterface* m_interface;
    QWebEngineView* m_view;

    QString getFileContents(const QString& path);
    void loadScriptFile(const QString& path, QWebEngineScript::InjectionPoint injectionPoint);
    void loadScriptString(const QString& data, QWebEngineScript::InjectionPoint injectionPoint);
};

#endif // WEBENGINEPLAYER_H
