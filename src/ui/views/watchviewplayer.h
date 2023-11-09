#ifndef WATCHVIEWPLAYER_H
#define WATCHVIEWPLAYER_H
#include "innertube/responses/video/playerresponse.h"

#ifdef QTTUBE_USE_MPV
#include "lib/media/media.h"
#include <QTimer>
#else
#include "ui/widgets/webengineplayer/webengineplayer.h"
#endif

class WatchViewPlayer : public QObject
{
    Q_OBJECT
public:
    WatchViewPlayer(QWidget* watchView, const QSize& maxSize);
    void play(const QString& videoId, int progress = 0);
    void seek(int progress);
    QWidget* widget();

    void calcAndSetSize(const QSize& maxSize);
    const QSize& size() const { return m_size; }

    void startTracking(const InnertubeEndpoints::PlayerResponse& playerResp);
    void stopTracking();
private:
    QSize m_size;
#ifndef QTTUBE_USE_MPV
    WebEnginePlayer* wePlayer;
#else
    Media* media;
    QPointer<QTimer> watchtimeTimer;
private slots:
    void mediaStateChanged(Media::State state);
    void volumeChanged(double volume);
#endif
signals:
    void progressChanged(double progress, double previousProgress);
};

#endif // WATCHVIEWPLAYER_H
