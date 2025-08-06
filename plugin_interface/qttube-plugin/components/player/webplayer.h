#pragma once
#include "fullscreennotification.h"
#include "player.h"
#include <QWebEngineFullScreenRequest>
#include <QWebEngineScript>
#include <QWebEngineView>

namespace QtTubePlugin
{
    class FullScreenWindow : public QWidget
    {
        Q_OBJECT
    public:
        explicit FullScreenWindow(QWebEngineView* view, QWidget* parent = nullptr);
        ~FullScreenWindow();
    protected:
        void resizeEvent(QResizeEvent* event) override;
    private:
        FullScreenNotification* m_notification;
        QRect m_oldGeometry;
        QWebEngineView* m_oldView;
        QWebEngineView* m_view;
    };

    // accessible via QWebChannel.objects.interface in injected script to enable some functionality in the program
    class WebChannelInterface : public QObject
    {
        Q_OBJECT
    public:
        using QObject::QObject;

        // use to give functionality to window title suffix and sleep state changing
        Q_INVOKABLE void emitNewState(Player::PlayerState state)
        { emit newState(state); }

        // use to give functionality to live chat
        Q_INVOKABLE void emitProgressChanged(qint64 progress, qint64 previousProgress)
        { emit progressChanged(progress, previousProgress); }

        // use to give functionality to i.e. in-player video recommendations
        Q_INVOKABLE void requestSwitchVideo(const QString& videoId)
        { emit switchVideoRequested(videoId); }
    signals:
        void newState(QtTubePlugin::Player::PlayerState state);
        void progressChanged(qint64 progress, qint64 previousProgress);
        void switchVideoRequested(const QString& videoId);
    };

    /*
     * QWebEngine-based player, equipped with:
     * QWebChannel support - accessible via m_channel member and inside of injected scripts (see Qt documentation)
     * WebChannelInterface - accessible with QWebChannel to send some signals back to the program
     * and with a handful of utility functions which you may find useful in injected scripts:
     * addStyle() - injects a stylesheet into the page
     * h264ify() - enforces serving of H.264 encoded content, if available. use to give H264 only setting functionality
     * waitForElement() - promise which will fire when an element satisfying a given query appears
     */
    class WebPlayer : public Player
    {
        Q_OBJECT
    public:
        explicit WebPlayer(QWidget* parent = nullptr);
    protected:
        QWebChannel* m_channel;
        WebChannelInterface* m_interface;
        QWebEngineView* m_view;

        void loadScriptData(
            const QString& data,
            QWebEngineScript::InjectionPoint injectionPoint = QWebEngineScript::Deferred,
            quint32 worldId = QWebEngineScript::MainWorld);
        void loadScriptFile(
            const QString& path,
            QWebEngineScript::InjectionPoint injectionPoint = QWebEngineScript::Deferred,
            quint32 worldId = QWebEngineScript::MainWorld);
        void loadStyleData(const QString& data);
        void loadStyleFile(const QString& path);
    protected slots:
        void fullScreenRequested(QWebEngineFullScreenRequest request);
    private:
        std::unique_ptr<FullScreenWindow> m_fullScreenWindow;
    };
}
