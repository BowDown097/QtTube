#pragma once
#include "plugins/pluginentry.h"
#include "preloaddata.h"

namespace ViewController
{
    void loadChannel(const QString& channelId, PluginEntry* plugin);
    void loadVideo(const QString& videoId, PluginEntry* plugin,
                   int progress = 0, PreloadData::WatchView* preload = nullptr,
                   bool continuePlayback = false);
    void unloadCurrent();
}
