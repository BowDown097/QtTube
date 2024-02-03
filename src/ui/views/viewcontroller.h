#ifndef VIEWCONTROLLER_H
#define VIEWCONTROLLER_H
#include <QString>

namespace PreloadData { class WatchView; }

class ViewController
{
public:
    static void loadChannel(const QString& channelId);
    static void loadVideo(const QString& videoId, int progress = 0, PreloadData::WatchView* preload = nullptr);
};

#endif // VIEWCONTROLLER_H
