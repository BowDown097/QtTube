#pragma once
#include "preloaddata.h"

class PluginEntry;

namespace ViewController
{
    void loadChannel(const QString& channelId, PluginEntry* plugin);
    void loadVideo(const QString& videoId, PluginEntry* plugin,
                   int progress = 0, PreloadData::WatchView* preload = nullptr,
                   bool continuePlayback = false);
    void unloadCurrent();
}
