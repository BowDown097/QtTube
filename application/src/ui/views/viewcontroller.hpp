#pragma once
#include "plugins/pluginentry.hpp"
#include "preloaddata.hpp"

namespace ViewController
{
    void loadChannel(const QString& channelId, PluginEntry* plugin);
    void loadVideo(const QString& videoId, PluginEntry* plugin,
                   int progress = 0, PreloadData::WatchView* preload = nullptr,
                   bool continuePlayback = false);
    void unloadCurrent();
}
