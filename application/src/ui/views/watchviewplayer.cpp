#include "watchviewplayer.h"
#include "qttubeapplication.h"
#include "ui/views/viewcontroller.h"
#include "utils/osutils.h"
#include "utils/uiutils.h"
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>

WatchViewPlayer::WatchViewPlayer(QWidget* watchView, const QSize& maxSize) : QObject(watchView)
{
    if (qtTubeApp->settings().externalPlayerPath.isEmpty())
    {
        if (PluginData* plugin = qtTubeApp->plugins().activePlugin(); plugin && plugin->playerFunc)
        {
            m_player = plugin->playerFunc(watchView);
            connect(m_player, &QtTubePlugin::Player::copyToClipboardRequested, this, &WatchViewPlayer::copyToClipboard);
            connect(m_player, &QtTubePlugin::Player::newState, this, &WatchViewPlayer::newState);
            connect(m_player, &QtTubePlugin::Player::progressChanged, this, &WatchViewPlayer::progressChanged);
            connect(m_player, &QtTubePlugin::Player::switchVideoRequested, this, &WatchViewPlayer::switchVideo);
        }
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

void WatchViewPlayer::copyToClipboard(const QString& text)
{
    UIUtils::copyToClipboard(text);
}

void WatchViewPlayer::newState(QtTubePlugin::Player::PlayerState state)
{
    auto setWindowTitleSuffix = [](const QString& suffix) {
        if (QMainWindow* mainWindow = UIUtils::getMainWindow())
        {
            static QRegularExpression suffixRegex(R"( \[(Playing|Paused)\]$)");
            mainWindow->setWindowTitle(mainWindow->windowTitle().remove(suffixRegex).append(' ').append(suffix));
        }
    };

    switch (state)
    {
    case QtTubePlugin::Player::State_Playing:
        OSUtils::suspendIdleSleep(true);
        setWindowTitleSuffix("[Playing]");
        break;
    case QtTubePlugin::Player::State_Paused:
        OSUtils::suspendIdleSleep(false);
        setWindowTitleSuffix("[Paused]");
        break;
    case QtTubePlugin::Player::State_Ended:
        OSUtils::suspendIdleSleep(false);
        break;
    default: break;
    }
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

void WatchViewPlayer::switchVideo(const QString& videoId)
{
    ViewController::loadVideo(videoId);
}

QWidget* WatchViewPlayer::widget()
{
    return m_player;
}
