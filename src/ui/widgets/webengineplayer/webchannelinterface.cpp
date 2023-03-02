#include "webchannelinterface.h"
#include "ui/views/watchview.h"

WebChannelInterface* WebChannelInterface::instance()
{
    if (!m_interface)
        m_interface = new WebChannelInterface;
    return m_interface;
}

void WebChannelInterface::switchWatchViewVideo(const QString& videoId)
{
    WatchView::instance()->hotLoadVideo(videoId);
}
