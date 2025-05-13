#include "youtubeplugin.h"
#include "innertube.h"
#include "localcache.h"
#include "settings/youtubesettings.h"
#include "utils/conversion.h"
#include "youtubeauth.h"

static QtTube::PluginMetadata Metadata = {
    .name = "YouTube",
    .description = "Built-in plugin for YouTube.",
    .image = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/YouTube_full-color_icon_%282017%29.svg/330px-YouTube_full-color_icon_%282017%29.svg.png",
    .author = "BowDown097",
    .url = "https://github.com/BowDown097/QtTube"
};
DECLARE_QTTUBE_PLUGIN(YouTubePlugin, Metadata, YouTubeSettings, YouTubeAuth)

YouTubeAuth* authPtr = static_cast<YouTubeAuth*>(auth());
YouTubeSettings* settingsPtr = static_cast<YouTubeSettings*>(settings());

template<typename T>
void addVideo(QList<QtTube::PluginVideo>& videoList, const T& video, bool useThumbnailFromData)
{
    if constexpr (std::same_as<T, InnertubeObjects::AdSlot>)
    {
        if (!settingsPtr->videoIsFiltered(video))
        {
            std::visit([useThumbnailFromData, &videoList](auto&& v) {
                videoList.append(convertVideo(Metadata, settingsPtr, v, useThumbnailFromData));
            }, video.fulfillmentContent.fulfilledLayout.renderingContent);
        }
    }
    else
    {
        if (!settingsPtr->videoIsFiltered(video))
            videoList.append(convertVideo(Metadata, settingsPtr, video, useThumbnailFromData));
    }
}

QtTube::HomeReply* YouTubePlugin::getHome()
{
    QtTube::HomeReply* pluginReply = QtTube::HomeReply::create();

    InnertubeReply<InnertubeEndpoints::BrowseHome>* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::BrowseHome>();
    QObject::connect(tubeReply, &InnertubeReply<InnertubeEndpoints::BrowseHome>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<InnertubeEndpoints::BrowseHome>::finished, [pluginReply](const InnertubeEndpoints::BrowseHome& endpoint) {
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

        emit pluginReply->finished(videoList);
    });

    return pluginReply;
}

void YouTubePlugin::init()
{
    LocalCache* cache = LocalCache::instance("yt-client");
    cache->setMaxSeconds(86400);

    if (const QByteArray cver = cache->value("cver"); !cver.isNull())
    {
        InnerTube::instance()->createClient(InnertubeClient::ClientType::WEB, cver);
    }
    else
    {
        cache->clear();
        InnerTube::instance()->createClient(InnertubeClient::ClientType::WEB, "2.20250421.01.00", true);
        cache->insert("cver", InnerTube::instance()->context()->client.clientVersion.toLatin1());
    }

    if (const CredentialSet* activeLogin = dynamic_cast<const CredentialSet*>(authPtr->activeLogin()))
    {
        authPtr->populateAuthStore(*activeLogin);
        if (InnerTube::instance()->hasAuthenticated())
            emit InnerTube::instance()->authStore()->authenticateSuccess();
    }
}
