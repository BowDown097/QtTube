#pragma once
#include <QObject>
#include <QSize>

class WebEnginePlayer;
namespace InnertubeEndpoints { struct PlayerResponse; }

class WatchViewPlayer : public QObject
{
    Q_OBJECT
public:
    enum class ScaleMode { Unset, NoScale, Scaled };

    WatchViewPlayer(QWidget* watchView, const QSize& maxSize);
    void calcAndSetSize(const QSize& maxSize);
    void play(const QString& videoId, int progress = 0);
    void seek(int progress);
    void startTracking(const InnertubeEndpoints::PlayerResponse& playerResp);
    QWidget* widget();

    const QSize& size() const { return m_size; }
    ScaleMode scaleMode() const { return m_scaleMode; }
private:
    WebEnginePlayer* m_player;
    ScaleMode m_scaleMode;
    QSize m_size;
signals:
    void progressChanged(double progress, double previousProgress);
    void scaleModeChanged(WatchViewPlayer::ScaleMode scaleMode);
};
