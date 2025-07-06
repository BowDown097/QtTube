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

    class WebChannelInterface : public QObject
    {
        Q_OBJECT
    public:
        using QObject::QObject;
        Q_INVOKABLE void emitNewState(Player::PlayerState state)
        { emit newState(state); }
        Q_INVOKABLE void emitProgressChanged(qint64 progress, qint64 previousProgress)
        { emit progressChanged(progress, previousProgress); }
        Q_INVOKABLE void requestCopyToClipboard(const QString& text)
        { emit copyToClipboardRequested(text); }
        Q_INVOKABLE void requestSwitchVideo(const QString& videoId)
        { emit switchVideoRequested(videoId); }
    signals:
        void copyToClipboardRequested(const QString& text);
        void newState(QtTubePlugin::Player::PlayerState state);
        void progressChanged(qint64 progress, qint64 previousProgress);
        void switchVideoRequested(const QString& videoId);
    };

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
    protected slots:
        void fullScreenRequested(QWebEngineFullScreenRequest request);
    private:
        std::unique_ptr<FullScreenWindow> m_fullScreenWindow;
    };
}
