#include "webchannelinterface.h"
#include "qttubeapplication.h"
#include "ui/views/viewcontroller.h"
#include "utils/osutils.h"
#include "utils/uiutils.h"
#include <QDesktopServices>
#include <QMainWindow>
#include <QRegularExpression>
#include <QUrl>

WebChannelInterface::WebChannelInterface(QObject* parent) : QObject(parent)
{
    connect(&qtTubeApp->settings(), &SettingsStore::preferredQualityChanged, &qtTubeApp->settings(), &SettingsStore::save);
    connect(&qtTubeApp->settings(), &SettingsStore::preferredVolumeChanged, &qtTubeApp->settings(), &SettingsStore::save);
}

void WebChannelInterface::copyToClipboard(const QString& text)
{
    UIUtils::copyToClipboard(text);
}

void WebChannelInterface::emitProgressChanged(double progress, double previousProgress)
{
    emit progressChanged(progress, previousProgress);
}

void WebChannelInterface::handleShare(const QString& href)
{
    QDesktopServices::openUrl(QUrl(href));
}

void WebChannelInterface::handleStateChange(PlayerState state)
{
    switch (state)
    {
    case STATE_PLAYING:
        OSUtils::suspendIdleSleep(true);
        setWindowTitleSuffix("[Playing]");
        break;
    case STATE_PAUSED:
        OSUtils::suspendIdleSleep(false);
        setWindowTitleSuffix("[Paused]");
        break;
    case STATE_ENDED:
        OSUtils::suspendIdleSleep(false);
        break;
    default: break;
    }
}

void WebChannelInterface::setWindowTitleSuffix(const QString& suffix)
{
    if (QMainWindow* mainWindow = UIUtils::getMainWindow())
    {
        static QRegularExpression suffixRegex(R"( \[(Playing|Paused)\]$)");
        mainWindow->setWindowTitle(mainWindow->windowTitle().remove(suffixRegex).append(' ').append(suffix));
    }
}

void WebChannelInterface::switchWatchViewVideo(const QString& videoId)
{
    ViewController::loadVideo(videoId);
}
