#include "replydata.h"
#include "conversion.h"
#include "innertube.h"
#include "youtubeplugin.h"

template<typename T>
void addVideo(QList<QtTube::PluginVideo>& videoList, const T& video, bool useThumbnailFromData)
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

QList<QtTube::PluginVideo> getHomeData(const InnertubeEndpoints::BrowseHome& endpoint)
{
    QList<QtTube::PluginVideo> videoList;

    // non-authenticated users will be under the IOS_UNPLUGGED client,
    // which serves thumbnails in an odd aspect ratio.
    bool useThumbnailFromData = InnerTube::instance()->hasAuthenticated();

    for (const InnertubeEndpoints::HomeResponseItem& item : endpoint.response.contents)
    {
        if (const auto* adSlot = std::get_if<InnertubeObjects::AdSlot>(&item))
        {
            addVideo(videoList, *adSlot, useThumbnailFromData);
        }
        else if (const auto* horizontalShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
        {
            for (const InnertubeObjects::Video& video : horizontalShelf->content.items)
                addVideo(videoList, video, useThumbnailFromData);
        }
        else if (const auto* lockup = std::get_if<InnertubeObjects::LockupViewModel>(&item))
        {
            addVideo(videoList, *lockup, useThumbnailFromData);
        }
        else if (const auto* richShelf = std::get_if<InnertubeObjects::HomeRichShelf>(&item))
        {
            for (const auto& itemVariant : richShelf->contents)
            {
                std::visit([&videoList, useThumbnailFromData](auto&& item) {
                    using ItemType = std::remove_cvref_t<decltype(item)>;
                    if constexpr (!innertube_is_any_v<ItemType, InnertubeObjects::MiniGameCardViewModel, InnertubeObjects::Post>)
                        addVideo(videoList, item, useThumbnailFromData);
                }, itemVariant);
            }
        }
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
        {
            addVideo(videoList, *video, useThumbnailFromData);
        }
    }

    return videoList;
}
