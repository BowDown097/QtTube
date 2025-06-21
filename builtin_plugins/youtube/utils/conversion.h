#pragma once
#include "innertube/innertubeexception.h"
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/notification/notification.h"
#include "innertube/objects/shelves/reelshelf.h"
#include "innertube/objects/video/compactvideo.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/watchnext/secondaryinfo/videoowner.h"
#include "innertube/responses/browse/homeresponse.h"
#include "youtubeplugin.h"

QtTube::PluginBadge convertBadge(const InnertubeObjects::MetadataBadge& badge);
QtTube::PluginChannel convertChannel(const InnertubeObjects::Channel& channel);
QtTube::PluginChannel convertChannel(
    const InnertubeObjects::VideoOwner& owner, const InnertubeObjects::SubscribeButton& subscribeButton);
QtTube::PluginException convertException(const InnertubeException& ex);
QtTube::LiveChatItem convertLiveChatItem(const QJsonValue& item);
QtTube::PluginNotification convertNotification(const InnertubeObjects::Notification& notification);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(
    const InnertubeObjects::HomeRichShelf& hrShelf, bool useThumbnailFromData = true);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(
    const InnertubeObjects::HorizontalVideoShelf& hShelf, bool useThumbnailFromData = true);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::ReelShelf& rShelf);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::StandardVideoShelf& sShelf);
QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::VerticalVideoShelf& vShelf);
QtTube::PluginSubscribeButton convertSubscribeButton(
    const InnertubeObjects::SubscribeButton& subscribeButton, const QString& countText);
QtTube::PluginVideo convertVideo(const InnertubeObjects::CompactVideo& compactVideo, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::DisplayAd& displayAd, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::LockupViewModel& lockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::Reel& reel, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::ShortsLockupViewModel& shortsLockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::Video& video, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::VideoDisplayButtonGroup& video, bool useThumbnailFromData);

template<typename T>
void addChannel(QList<T>& channelList, const QtTube::PluginChannel& channel)
{
    if (!g_settings->channelIsFiltered(channel.channelId))
        channelList.append(channel);
}

template<typename T>
void addNotification(QList<T>& notificationList, const QtTube::PluginNotification& notification)
{
    if (!g_settings->channelIsFiltered(notification.channelId))
        notificationList.append(notification);
}

template<typename T, typename... Ts>
void addShelf(QList<T>& shelfList, const QtTube::PluginShelf<Ts...>& shelf)
{
    if (!shelf.contents.isEmpty())
        shelfList.append(shelf);
}

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
