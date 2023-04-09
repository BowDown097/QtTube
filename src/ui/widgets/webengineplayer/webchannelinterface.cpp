#include "webchannelinterface.h"
#include "ui/views/viewcontroller.h"

void WebChannelInterface::switchWatchViewVideo(const QString& videoId)
{
    ViewController::loadVideo(videoId);
}
