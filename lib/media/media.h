#ifndef MEDIA_H
#define MEDIA_H

#include <QtCore>
#ifndef MEDIA_AUDIOONLY
#include <QtWidgets>
#endif

class Media : public QObject {
    Q_OBJECT

public:
    enum State {
        StoppedState,
        LoadingState,
        BufferingState,
        PlayingState,
        PausedState,
        ErrorState
    };
    Q_ENUM(State)

    Media(QObject *parent = nullptr) : QObject(parent) {
        qRegisterMetaType<Media::State>("Media::State");
    }
    virtual void setAudioOnly(bool value) = 0;
#ifndef MEDIA_AUDIOONLY
    virtual void setRenderer(const QString &name) = 0;
    virtual QWidget *videoWidget() = 0;
    virtual void playSeparateAudioAndVideo(const QString &video, const QString &audio) = 0;
    virtual void snapshot() = 0;
#endif
    virtual void init() = 0;

    virtual Media::State state() const = 0;

    virtual void play(const QString &file) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(qint64 ms) = 0;
    virtual void relativeSeek(qint64 ms) = 0;
    virtual QString file() const = 0;

    virtual void setBufferMilliseconds(qint64 value) = 0;
    virtual void setUserAgent(const QString &value) = 0;

    virtual void enqueue(const QString &file) = 0;
    virtual void clearQueue() = 0;
    virtual bool hasQueue() const = 0;

    virtual qint64 position() const = 0;
    virtual qint64 duration() const = 0;
    virtual qint64 remainingTime() const = 0;

    virtual qreal volume() const = 0;
    virtual void setVolume(qreal value) = 0;

    virtual bool volumeMuted() const = 0;
    virtual void setVolumeMuted(bool value) = 0;

    virtual QString errorString() const = 0;

signals:
    void error(const QString &message);
    void sourceChanged();
    void bufferStatus(qreal value);
    void loaded();
    void started();
    void stopped();
    void paused(bool p);
    void stateChanged(Media::State state);
    void positionChanged(qint64 ms);
    void aboutToFinish();
    void finished();
    void volumeChanged(qreal value);
    void volumeMutedChanged(bool value);
#ifndef MEDIA_AUDIOONLY
    void snapshotReady(const QImage &image);
#endif
};

#endif // MEDIA_H
