#include "replydata.h"
#include "conversion.h"
#include "innertube.h"
#include "youtubeplugin.h"

template<typename T, typename U>
void addVideo(QList<U>& videoList, const T& video, bool useThumbnailFromData)
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

QtTube::HomeData getHomeData(const InnertubeEndpoints::BrowseHome& endpoint)
{
    QtTube::HomeData result;

    // non-authenticated users will be under the IOS_UNPLUGGED client,
    // which serves thumbnails in an odd aspect ratio.
    bool useThumbnailFromData = InnerTube::instance()->hasAuthenticated();

    for (const InnertubeEndpoints::HomeResponseItem& item : endpoint.response.contents)
    {
        if (const auto* adSlot = std::get_if<InnertubeObjects::AdSlot>(&item))
        {
            addVideo(result, *adSlot, useThumbnailFromData);
        }
        else if (const auto* hShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
        {
            QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
            videoShelf.title = hShelf->title.text;
            if (const InnertubeObjects::GenericThumbnail* bestThumb = hShelf->thumbnail.bestQuality())
                videoShelf.iconUrl = bestThumb->url;

            for (const InnertubeObjects::Video& video : hShelf->content.items)
                addVideo(videoShelf.contents, video, useThumbnailFromData);

            result.append(videoShelf);
        }
        else if (const auto* lockup = std::get_if<InnertubeObjects::LockupViewModel>(&item))
        {
            addVideo(result, *lockup, useThumbnailFromData);
        }
        else if (const auto* rShelf = std::get_if<InnertubeObjects::HomeRichShelf>(&item))
        {
            QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
            videoShelf.isDividerHidden = rShelf->isBottomDividerHidden;
            videoShelf.subtitle = rShelf->subtitle.text;
            videoShelf.title = rShelf->title.text;
            if (const InnertubeObjects::GenericThumbnail* bestThumb = rShelf->thumbnail.bestQuality())
                videoShelf.iconUrl = bestThumb->url;

            for (const auto& itemVariant : rShelf->contents)
            {
                std::visit([&videoShelf, useThumbnailFromData](auto&& item) {
                    using ItemType = std::remove_cvref_t<decltype(item)>;
                    if constexpr (!innertube_is_any_v<ItemType, InnertubeObjects::MiniGameCardViewModel, InnertubeObjects::Post>)
                        addVideo(videoShelf.contents, item, useThumbnailFromData);
                }, itemVariant);
            }

            result.append(videoShelf);
        }
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
        {
            addVideo(result, *video, useThumbnailFromData);
        }
    }

    return result;
}
