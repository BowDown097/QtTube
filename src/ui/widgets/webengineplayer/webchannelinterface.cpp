#include "webchannelinterface.h"
#include "qttubeapplication.h"
#include "ui/views/viewcontroller.h"
#include "utils/osutils.h"
#include <QMainWindow>
#include <QRegularExpression>

WebChannelInterface::WebChannelInterface(QObject* parent) : QObject(parent)
{
    connect(&qtTubeApp->settings(), &SettingsStore::preferredQualityChanged, &qtTubeApp->settings(), &SettingsStore::save);
    connect(&qtTubeApp->settings(), &SettingsStore::preferredVolumeChanged, &qtTubeApp->settings(), &SettingsStore::save);
}

void setWindowTitleSuffix(const QString& suffix)
{
    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow()))
    {
        static QRegularExpression suffixRegex(R"( \[(Playing|Paused)\]$)");
        mainWindow->setWindowTitle(mainWindow->windowTitle().remove(suffixRegex).append(' ').append(suffix));
    }
}

void WebChannelInterface::emitProgressChanged(double progress, double previousProgress)
{
    emit progressChanged(progress, previousProgress);
}

void WebChannelInterface::handleStateChange(PlayerState state)
{
    switch (state)
    {
    case STATE_PLAYING:
        OSUtils::toggleIdleSleep(true);
        setWindowTitleSuffix("[Playing]");
        break;
    case STATE_PAUSED:
        OSUtils::toggleIdleSleep(false);
        setWindowTitleSuffix("[Paused]");
        break;
    case STATE_ENDED:
        OSUtils::toggleIdleSleep(false);
        break;
    default: break;
    }
}

void WebChannelInterface::switchWatchViewVideo(const QString& videoId)
{
    ViewController::loadVideo(videoId);
}
