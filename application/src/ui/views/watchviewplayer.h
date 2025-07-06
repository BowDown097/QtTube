#pragma once
#include "qttube-plugin/components/player/player.h"

class WatchViewPlayer : public QObject
{
    Q_OBJECT
public:
    enum class ScaleMode { Unset, NoScale, Scaled };

    WatchViewPlayer(QWidget* watchView, const QSize& maxSize);
    void calcAndSetSize(const QSize& maxSize);
    void play(const QString& videoId, int progress = 0);
    void seek(int progress);
    QWidget* widget();

    QSize size() const { return m_size; }
    ScaleMode scaleMode() const { return m_scaleMode; }
private:
    QtTubePlugin::Player* m_player{};
    ScaleMode m_scaleMode;
    QSize m_size;
private slots:
    void copyToClipboard(const QString& text);
    void newState(QtTubePlugin::Player::PlayerState state);
    void switchVideo(const QString& videoId);
signals:
    void progressChanged(qint64 progress, qint64 previousProgress);
    void scaleModeChanged(WatchViewPlayer::ScaleMode scaleMode);
};
