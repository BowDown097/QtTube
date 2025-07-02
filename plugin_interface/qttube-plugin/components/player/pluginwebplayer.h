#pragma once
#include "fullscreennotification.h"
#include "pluginplayer.h"
#include <QWebEngineFullScreenRequest>
#include <QWebEngineScript>
#include <QWebEngineView>

namespace QtTube
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
        Q_INVOKABLE void emitNewState(PluginPlayer::PlayerState state)
        { emit newState(state); }
        Q_INVOKABLE void emitProgressChanged(qint64 progress, qint64 previousProgress)
        { emit progressChanged(progress, previousProgress); }
        Q_INVOKABLE void requestCopyToClipboard(const QString& text)
        { emit copyToClipboardRequested(text); }
        Q_INVOKABLE void requestSwitchVideo(const QString& videoId)
        { emit switchVideoRequested(videoId); }
    signals:
        void copyToClipboardRequested(const QString& text);
        void newState(QtTube::PluginPlayer::PlayerState state);
        void progressChanged(qint64 progress, qint64 previousProgress);
        void switchVideoRequested(const QString& videoId);
    };

    class PluginWebPlayer : public PluginPlayer
    {
        Q_OBJECT
    public:
        explicit PluginWebPlayer(QWidget* parent = nullptr);
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
