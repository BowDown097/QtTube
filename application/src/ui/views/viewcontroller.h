#pragma once
#include "preloaddata.h"

struct PluginData;

namespace ViewController
{
    void loadChannel(const QString& channelId, PluginData* plugin);
    void loadVideo(const QString& videoId, PluginData* plugin,
                   int progress = 0, PreloadData::WatchView* preload = nullptr,
                   bool continuePlayback = false);
    void unloadCurrent();
}
