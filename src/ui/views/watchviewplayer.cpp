#include "watchviewplayer.h"
#include "innertube.h"
#include "ui/widgets/webengineplayer/webengineplayer.h"

WatchViewPlayer::WatchViewPlayer(QWidget* watchView, const QSize& maxSize) : QObject(watchView)
{
    m_player = new WebEnginePlayer(watchView);
    m_player->setAuthStore(InnerTube::instance()->authStore());
    m_player->setContext(InnerTube::instance()->context());
    connect(m_player, &WebEnginePlayer::progressChanged, this, &WatchViewPlayer::progressChanged);
    calcAndSetSize(maxSize);
}

void WatchViewPlayer::calcAndSetSize(const QSize& maxSize)
{
    WatchViewPlayer::ScaleMode currentScaleMode = m_scaleMode;
    int playerWidth = maxSize.width();
    int playerHeight = playerWidth * 9/16;
    int maxHeightBeforeScale = maxSize.height() - 150;

    if (playerHeight > maxHeightBeforeScale)
    {
        playerWidth = std::min(maxHeightBeforeScale * 16/9, maxSize.width() - 500);
        playerHeight = playerWidth * 9/16;
        m_scaleMode = WatchViewPlayer::ScaleMode::Scaled;
    }
    else
    {
        m_scaleMode = WatchViewPlayer::ScaleMode::NoScale;
    }

    if (m_scaleMode != currentScaleMode)
        emit scaleModeChanged(m_scaleMode);

    m_size = QSize(playerWidth, playerHeight);
    m_player->setFixedSize(m_size);
}

void WatchViewPlayer::play(const QString& videoId, int progress)
{
    m_player->play(videoId, progress);
}

void WatchViewPlayer::seek(int progress)
{
    m_player->seek(progress);
}

void WatchViewPlayer::startTracking(const InnertubeEndpoints::PlayerResponse& playerResp)
{
    m_player->setPlayerResponse(playerResp);
}

QWidget* WatchViewPlayer::widget()
{
    return m_player;
}
