#pragma once
#include <QString>

namespace PreloadData { struct WatchView; }

class ViewController
{
public:
    static void loadChannel(const QString& channelId);
    static void loadVideo(const QString& videoId, int progress = 0, PreloadData::WatchView* preload = nullptr);
};
