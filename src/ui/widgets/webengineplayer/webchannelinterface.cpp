#include "webchannelinterface.h"
#include "qttubeapplication.h"
#include "ui/views/viewcontroller.h"
#include "utils/osutils.h"

WebChannelInterface::WebChannelInterface(QObject* parent) : QObject(parent)
{
    connect(&qtTubeApp->settings(), &SettingsStore::preferredQualityChanged, &qtTubeApp->settings(), &SettingsStore::save);
    connect(&qtTubeApp->settings(), &SettingsStore::preferredVolumeChanged, &qtTubeApp->settings(), &SettingsStore::save);
}

void WebChannelInterface::emitProgressChanged(double progress, double previousProgress)
{
    emit progressChanged(progress, previousProgress);
}

void WebChannelInterface::handleStateChange(PlayerState state)
{
    if (state == STATE_PLAYING)
        OSUtils::toggleIdleSleep(true);
    else if (state == STATE_PAUSED || state == STATE_ENDED)
        OSUtils::toggleIdleSleep(false);
}

void WebChannelInterface::switchWatchViewVideo(const QString& videoId)
{
    ViewController::loadVideo(videoId);
}
