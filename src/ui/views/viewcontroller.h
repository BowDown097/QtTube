#ifndef VIEWCONTROLLER_H
#define VIEWCONTROLLER_H
#include <QStackedWidget>

class ViewController
{
public:
    static void loadChannel(const QString& channelId);
    static void loadVideo(const QString& videoId, int progress = 0);
};

#endif // VIEWCONTROLLER_H
