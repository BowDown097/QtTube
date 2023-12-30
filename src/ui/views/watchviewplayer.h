#ifndef WATCHVIEWPLAYER_H
#define WATCHVIEWPLAYER_H
#include <QObject>
#include <QSize>

#ifdef QTTUBE_USE_MPV
#include "lib/media/media.h"
#include <QTimer>
#else
class WebEnginePlayer;
#endif

namespace InnertubeEndpoints { class PlayerResponse; }

enum class PlayerScaleMode { Unset, NoScale, Scaled };

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

    PlayerScaleMode scaleMode() const { return m_scaleMode; }

    void startTracking(const InnertubeEndpoints::PlayerResponse& playerResp);
    void stopTracking();
private:
    PlayerScaleMode m_scaleMode;
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
    void scaleModeChanged(PlayerScaleMode scaleMode);
};

#endif // WATCHVIEWPLAYER_H
