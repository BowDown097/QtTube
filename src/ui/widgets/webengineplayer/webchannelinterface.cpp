#include "webchannelinterface.h"
#include "ui/views/viewcontroller.h"

WebChannelInterface* WebChannelInterface::instance()
{
    if (!m_interface)
        m_interface = new WebChannelInterface;
    return m_interface;
}

void WebChannelInterface::switchWatchViewVideo(const QString& videoId)
{
    ViewController::loadVideo(videoId);
}
