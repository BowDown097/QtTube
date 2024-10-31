#pragma once
#include <QString>

namespace PreloadData { struct WatchView; }

namespace ViewController
{
    void loadChannel(const QString& channelId);
    void loadVideo(const QString& videoId, int progress = 0, PreloadData::WatchView* preload = nullptr);
}
