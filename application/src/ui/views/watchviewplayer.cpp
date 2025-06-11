#include "watchviewplayer.h"
#include "innertube.h"
#include "qttubeapplication.h"
#include "ui/widgets/webengineplayer/webengineplayer.h"
#include <QLabel>
#include <QMessageBox>
#include <QProcess>

WatchViewPlayer::WatchViewPlayer(QWidget* watchView, const QSize& maxSize) : QObject(watchView)
{
    if (qtTubeApp->settings().externalPlayerPath.isEmpty())
    {
        m_player = new WebEnginePlayer(watchView);
        m_player->setAuthStore(InnerTube::instance()->authStore());
        m_player->setContext(InnerTube::instance()->context());
        connect(m_player, &WebEnginePlayer::progressChanged, this, &WatchViewPlayer::progressChanged);
    }

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

    if (m_player)
        m_player->setFixedSize(m_size);
}

void WatchViewPlayer::play(const QString& videoId, int progress)
{
    if (QString playerPath = qtTubeApp->settings().externalPlayerPath; !playerPath.isEmpty())
    {
        // using splitCommand + start instead of startCommand for Qt 5.15 support
        QProcess* process = new QProcess(this);
        QStringList args = process->splitCommand(playerPath
            .replace("%U", "https://www.youtube.com/watch?v=" + videoId)
            .replace("%P", QString::number(progress)));
        const QString program = args.takeFirst();
        process->start(program, args);
    }
    else
    {
        m_player->play(videoId, progress);
    }
}

void WatchViewPlayer::seek(int progress)
{
    if (m_player)
        m_player->seek(progress);
}

void WatchViewPlayer::showSharePanel()
{
    if (m_player)
        m_player->showSharePanel();
}

QWidget* WatchViewPlayer::widget()
{
    return m_player;
}
