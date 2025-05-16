#include "replydata.h"
#include "conversion.h"
#include "innertube.h"

template<typename T, typename... Ts>
void addShelf(QList<T>& shelfList, const QtTube::PluginShelf<Ts...>& shelf)
{
    if (!shelf.contents.isEmpty())
        shelfList.append(shelf);
}

QtTube::BrowseData getHomeData(const InnertubeEndpoints::BrowseHome& endpoint)
{
    QtTube::BrowseData result;

    // non-authenticated users will be under the IOS_UNPLUGGED client,
    // which serves thumbnails in an odd aspect ratio.
    bool useThumbnailFromData = InnerTube::instance()->hasAuthenticated();

    for (const InnertubeEndpoints::HomeResponseItem& item : endpoint.response.contents)
    {
        if (const auto* adSlot = std::get_if<InnertubeObjects::AdSlot>(&item))
            addVideo(result, *adSlot, useThumbnailFromData);
        else if (const auto* hrShelf = std::get_if<InnertubeObjects::HomeRichShelf>(&item))
            addShelf(result, convertShelf(*hrShelf, useThumbnailFromData));
        else if (const auto* hShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
            addShelf(result, convertShelf(*hShelf, useThumbnailFromData));
        else if (const auto* lockup = std::get_if<InnertubeObjects::LockupViewModel>(&item))
            addVideo(result, *lockup, useThumbnailFromData);
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
            addVideo(result, *video, useThumbnailFromData);
    }

    return result;
}

QtTube::BrowseData getTrendingData(const InnertubeEndpoints::BrowseTrending& endpoint)
{
    QtTube::BrowseData result;

    for (const InnertubeEndpoints::TrendingResponseItem& item : endpoint.response.contents)
    {
        if (const auto* hShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
            addShelf(result, convertShelf(*hShelf));
        else if (const auto* rShelf = std::get_if<InnertubeObjects::ReelShelf>(&item))
            addShelf(result, convertShelf(*rShelf));
        else if (const auto* sShelf = std::get_if<InnertubeObjects::StandardVideoShelf>(&item))
            addShelf(result, convertShelf(*sShelf));
    }

    return result;
}
