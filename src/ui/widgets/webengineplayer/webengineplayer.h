#pragma once
#include <QWebEngineFullScreenRequest>
#include <QWebEngineScript>
#include <QWidget>

namespace InnertubeEndpoints { struct PlayerResponse; }

class FullScreenWindow;
class InnertubeAuthStore;
struct InnertubeContext;
class PlayerInterceptor;
class QWebEngineFullScreenRequest;
class QWebEngineView;
class WebChannelInterface;

class WebEnginePlayer : public QWidget
{
    Q_OBJECT
public:
    explicit WebEnginePlayer(QWidget* parent = nullptr);
    void setAuthStore(InnertubeAuthStore* authStore);
    void setContext(InnertubeContext* context);
    void setPlayerResponse(const InnertubeEndpoints::PlayerResponse& resp);
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
signals:
    void progressChanged(double progress, double previousProgress);
};
