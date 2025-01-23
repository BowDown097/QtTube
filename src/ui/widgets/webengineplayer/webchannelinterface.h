#pragma once
#include <QObject>

class WebChannelInterface : public QObject
{
    Q_OBJECT
public:
    enum PlayerState
    {
        STATE_UNSTARTED = -1,
        STATE_ENDED = 0,
        STATE_PLAYING = 1,
        STATE_PAUSED = 2,
        STATE_BUFFERING = 3,
        STATE_CUED = 5
    };

    explicit WebChannelInterface(QObject* parent = nullptr);
    Q_INVOKABLE void copyToClipboard(const QString& text);
    Q_INVOKABLE void emitProgressChanged(double progress, double previousProgress);
    Q_INVOKABLE void handleShare(const QString& href);
    Q_INVOKABLE void handleStateChange(PlayerState state);
    Q_INVOKABLE void switchWatchViewVideo(const QString& videoId);
private:
    void setWindowTitleSuffix(const QString& suffix);
signals:
    void progressChanged(double progress, double previousProgress);
};
