#ifndef WEBCHANNELINTERFACE_H
#define WEBCHANNELINTERFACE_H
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
    Q_INVOKABLE void emitProgressChanged(double progress, double previousProgress);
    Q_INVOKABLE void handleStateChange(PlayerState state);
public slots:
    void switchWatchViewVideo(const QString& videoId);
signals:
    void progressChanged(double progress, double previousProgress);
};

#endif // WEBCHANNELINTERFACE_H
