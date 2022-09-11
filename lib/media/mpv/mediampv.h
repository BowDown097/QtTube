#ifndef MEDIAMPV_H
#define MEDIAMPV_H

#include <QtCore>

#include "../media.h"
#include <mpv/client.h>

class MediaMPV : public Media {
    Q_OBJECT

public:
    MediaMPV(QObject *parent = nullptr);

    void setAudioOnly(bool value);
#ifndef MEDIA_AUDIOONLY
    void setRenderer(const QString &name);
    QWidget *videoWidget();
    void playSeparateAudioAndVideo(const QString &video, const QString &audio);
    void snapshot();
#endif
    void init();
    Media::State state() const;
    void play(const QString &file);
    void play();
    void pause();
    void stop();
    void seek(qint64 ms);
    void relativeSeek(qint64 ms);
    QString file() const;
    void setBufferMilliseconds(qint64 value);
    void setUserAgent(const QString &value);
    void enqueue(const QString &file);
    void clearQueue();
    bool hasQueue() const;
    qint64 position() const;
    qint64 duration() const;
    qint64 remainingTime() const;
    qreal volume() const;
    void setVolume(qreal value);
    bool volumeMuted() const;
    void setVolumeMuted(bool value);
    QString errorString() const;

private slots:
    void onMpvEvents();
    void checkAboutToFinish(qint64 position);

signals:
    void mpvEvents();

private:
    void handleMpvEvent(mpv_event *event);
    void sendCommand(const char *args[]);
    void setState(Media::State value);
    void clearTrackState();

    QWidget *widget;
    mpv_handle *mpv;
    Media::State currentState = Media::StoppedState;
    bool aboutToFinishEmitted = false;
    QString lastErrorString;

    QString audioFileToAdd;
};

#endif // MEDIAMPV_H
