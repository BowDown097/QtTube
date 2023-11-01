#include "webchannelinterface.h"
#include "qttubeapplication.h"
#include "ui/views/viewcontroller.h"

WebChannelInterface::WebChannelInterface(QObject* parent) : QObject(parent)
{
    connect(&qtTubeApp->settings(), &SettingsStore::preferredQualityChanged, &qtTubeApp->settings(), &SettingsStore::save);
    connect(&qtTubeApp->settings(), &SettingsStore::preferredVolumeChanged, &qtTubeApp->settings(), &SettingsStore::save);
}

void WebChannelInterface::switchWatchViewVideo(const QString& videoId)
{
    ViewController::loadVideo(videoId);
}
