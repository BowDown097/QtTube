#pragma once
#include "innertube/innertubeexception.h"
#include "innertube/objects/notification/notification.h"
#include "innertube/objects/shelves/reelshelf.h"
#include "innertube/objects/video/compactvideo.h"
#include "innertube/objects/video/reel.h"
#include "innertube/responses/browse/homeresponse.h"
#include "youtubeplugin.h"

QtTube::PluginException convertException(const InnertubeException& ex);
QtTube::PluginNotification convertNotification(const InnertubeObjects::Notification& notification);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(
    const InnertubeObjects::HomeRichShelf& hrShelf, bool useThumbnailFromData = true);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(
    const InnertubeObjects::HorizontalVideoShelf& hShelf, bool useThumbnailFromData = true);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::ReelShelf& rShelf);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::StandardVideoShelf& sShelf);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::VerticalVideoShelf& vShelf);
QtTube::PluginVideo convertVideo(const InnertubeObjects::CompactVideo& compactVideo, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::DisplayAd& displayAd, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::LockupViewModel& lockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::Reel& reel, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::ShortsLockupViewModel& shortsLockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::Video& video, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::VideoDisplayButtonGroup& video, bool useThumbnailFromData);

template<typename T, typename U>
void addVideo(QList<U>& videoList, const T& video, bool useThumbnailFromData = true)
{
    if constexpr (std::same_as<T, InnertubeObjects::AdSlot>)
    {
        if (!g_settings->videoIsFiltered(video))
        {
            std::visit([useThumbnailFromData, &videoList](auto&& v) {
                videoList.append(convertVideo(v, useThumbnailFromData));
            }, video.fulfillmentContent.fulfilledLayout.renderingContent);
        }
    }
    else
    {
        if (!g_settings->videoIsFiltered(video))
            videoList.append(convertVideo(video, useThumbnailFromData));
    }
}
