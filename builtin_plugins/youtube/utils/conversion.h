#pragma once
#include "innertube/innertubeexception.h"
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/channel/channelc4header.h"
#include "innertube/objects/channel/channelpageheader.h"
#include "innertube/objects/notification/notification.h"
#include "innertube/objects/shelves/reelshelf.h"
#include "innertube/objects/video/compactvideo.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/watchnext/secondaryinfo/videoowner.h"
#include "innertube/responses/browse/homeresponse.h"
#include "youtubeplugin.h"

QtTubePlugin::Badge convertBadge(const InnertubeObjects::MetadataBadge& badge);
QtTubePlugin::Channel convertChannel(const InnertubeObjects::Channel& channel);
QtTubePlugin::Channel convertChannel(
    const InnertubeObjects::VideoOwner& owner, const InnertubeObjects::SubscribeButton& subscribeButton);
QtTubePlugin::ChannelHeader convertChannelHeader(const InnertubeObjects::ChannelC4Header& header);
QtTubePlugin::ChannelHeader convertChannelHeader(
    const InnertubeObjects::ChannelPageHeader& header, const QList<InnertubeObjects::EntityMutation>& mutations);
QtTubePlugin::Exception convertException(const InnertubeException& ex);
QtTubePlugin::LiveChatItem convertLiveChatItem(const QJsonValue& item);
QtTubePlugin::Notification convertNotification(const InnertubeObjects::Notification& notification);
QtTubePlugin::Shelf<QtTubePlugin::Video> convertShelf(
    const InnertubeObjects::HomeRichShelf& hrShelf, bool useThumbnailFromData = true);
QtTubePlugin::Shelf<QtTubePlugin::Video> convertShelf(
    const InnertubeObjects::HorizontalVideoShelf& hShelf, bool useThumbnailFromData = true);
QtTubePlugin::Shelf<QtTubePlugin::Video> convertShelf(const InnertubeObjects::ReelShelf& rShelf);
QtTubePlugin::Shelf<QtTubePlugin::Video> convertShelf(const InnertubeObjects::StandardVideoShelf& sShelf);
QtTubePlugin::Shelf<QtTubePlugin::Video> convertShelf(const InnertubeObjects::VerticalVideoShelf& vShelf);
QtTubePlugin::SubscribeButton convertSubscribeButton(
    const InnertubeObjects::ButtonViewModel& button, const QString& countText);
QtTubePlugin::SubscribeButton convertSubscribeButton(
    const InnertubeObjects::SubscribeButton& subscribeButton, const QString& countText);
QtTubePlugin::SubscribeButton convertSubscribeButton(
    const InnertubeObjects::SubscribeButtonViewModel& subscribeButton, const QString& countText, bool subscribed);
QtTubePlugin::ChannelTabData convertTab(const QJsonValue& tabRenderer, std::any& continuationData);
QtTubePlugin::Video convertVideo(const InnertubeObjects::CompactVideo& compactVideo, bool useThumbnailFromData);
QtTubePlugin::Video convertVideo(const InnertubeObjects::DisplayAd& displayAd, bool useThumbnailFromData);
QtTubePlugin::Video convertVideo(const InnertubeObjects::LockupViewModel& lockup, bool useThumbnailFromData);
QtTubePlugin::Video convertVideo(const InnertubeObjects::Reel& reel, bool useThumbnailFromData);
QtTubePlugin::Video convertVideo(const InnertubeObjects::ShortsLockupViewModel& shortsLockup, bool useThumbnailFromData);
QtTubePlugin::Video convertVideo(const InnertubeObjects::Video& video, bool useThumbnailFromData);
QtTubePlugin::Video convertVideo(const InnertubeObjects::VideoDisplayButtonGroup& video, bool useThumbnailFromData);

template<typename T>
void addChannel(QList<T>& channelList, const QtTubePlugin::Channel& channel)
{
    if (!g_settings->channelIsFiltered(channel.channelId))
        channelList.append(channel);
}

template<typename T>
void addNotification(QList<T>& notificationList, const QtTubePlugin::Notification& notification)
{
    if (!g_settings->channelIsFiltered(notification.channelId))
        notificationList.append(notification);
}

template<typename T, typename... Ts>
void addShelf(QList<T>& shelfList, const QtTubePlugin::Shelf<Ts...>& shelf)
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
