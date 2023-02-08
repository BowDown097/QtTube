#include "webchannelmethods.h"
#include "ui/views/watchview.h"

WebChannelMethods* WebChannelMethods::instance()
{
    if (!m_methods)
        m_methods = new WebChannelMethods;
    return m_methods;
}

void WebChannelMethods::switchWatchViewVideo(const QString& videoId)
{
    WatchView::instance()->hotLoadVideo(videoId);
}
