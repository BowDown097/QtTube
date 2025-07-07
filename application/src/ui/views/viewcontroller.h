#pragma once
#include "preloaddata.h"

namespace ViewController
{
    void loadChannel(const QString& channelId);
    void loadVideo(
        const QString& videoId, int progress = 0,
        PreloadData::WatchView* preload = nullptr, bool continuePlayback = false);
    void unloadCurrent();
}
