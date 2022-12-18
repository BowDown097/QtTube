#ifndef BROWSEHELPER_TPP
#define BROWSEHELPER_TPP
#include "innertube.h"
#include <QMessageBox>
#include <QScrollBar>

template<typename T> requires std::derived_from<T, InnertubeEndpoints::BaseEndpoint>
void BrowseHelper::tryContinuation(int value, QListWidget* widget, const QString& data, int threshold)
{
    if (value < widget->verticalScrollBar()->maximum() - threshold || continuationOngoing || InnerTube::instance().context()->client.visitorData.isEmpty() || widget->count() == 0)
        return;

    continuationOngoing = true;

    try
    {
        T newData = InnerTube::instance().get<T>(data, continuationToken);
        if constexpr (std::is_same_v<T, InnertubeEndpoints::Search>)
        {
            setupChannelList(newData.response.channels, widget);
            setupVideoList(newData.response.videos, widget);
        }
        else if constexpr (std::is_same_v<T, InnertubeEndpoints::GetNotificationMenu>)
        {
            setupNotificationList(newData.response.notifications, widget);
        }
        else
        {
            setupVideoList(newData.response.videos, widget);
        }

        continuationToken = newData.continuationToken;
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(nullptr, "Failed to get continuation browsing info", ie.message());
    }

    continuationOngoing = false;
}

#endif // BROWSEHELPER_TPP
