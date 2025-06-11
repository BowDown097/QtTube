#include "replydata.h"
#include "conversion.h"
#include "httprequest.h"
#include "innertube.h"
#include "utils/stringutils.h"
#include "utils/tubeutils.h"

QtTube::BrowseData getHistoryData(const InnertubeEndpoints::HistoryResponse& response)
{
    QtTube::BrowseData result;
    for (const InnertubeObjects::Video& video : response.videos)
        addVideo(result, video);
    return result;
}

QtTube::BrowseData getHomeData(const InnertubeEndpoints::HomeResponse& response)
{
    QtTube::BrowseData result;

    // non-authenticated users will be under the IOS_UNPLUGGED client,
    // which serves thumbnails in an odd aspect ratio.
    bool useThumbnailFromData = InnerTube::instance()->hasAuthenticated();

    for (const InnertubeEndpoints::HomeResponseItem& item : response.contents)
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

void getNextData(QtTube::VideoData& data, const InnertubeEndpoints::NextResponse& response)
{
    const InnertubeObjects::VideoPrimaryInfo& primaryInfo = response.contents.results.primaryInfo;
    const InnertubeObjects::VideoSecondaryInfo& secondaryInfo = response.contents.results.secondaryInfo;

    data.channel = convertChannel(secondaryInfo.owner, secondaryInfo.subscribeButton);
    data.viewCountText = g_settings->condensedCounts && !primaryInfo.viewCount.isLive
        ? primaryInfo.viewCount.extraShortViewCount.text + " views"
        : primaryInfo.viewCount.viewCount.text;
    data.videoId = response.videoId;

    const InnertubeObjects::LikeDislikeViewModel& likeDislikeViewModel = primaryInfo.videoActions.segmentedLikeDislikeButtonViewModel;
    const QString& likeStatus = likeDislikeViewModel.likeButtonViewModel.likeStatus;

    if (likeStatus == "LIKE")
        data.likeStatus = QtTube::VideoData::LikeStatus::Liked;
    else if (likeStatus == "DISLIKE")
        data.likeStatus = QtTube::VideoData::LikeStatus::Disliked;

    const InnertubeObjects::ButtonViewModel& likeViewModel = likeDislikeViewModel.likeButtonViewModel.toggleButtonViewModel.defaultButtonViewModel;
    const InnertubeObjects::ButtonViewModel& toggledLikeViewModel = likeDislikeViewModel.likeButtonViewModel.toggleButtonViewModel.toggledButtonViewModel;

    QString fullLikeCount = StringUtils::extractDigits(likeViewModel.accessibilityText);
    data.likeCountText = g_settings->condensedCounts ? likeViewModel.title : fullLikeCount;

    const QJsonValue defaultLikeEndpoint = likeViewModel.onTap["serialCommand"]["commands"][1]["innertubeCommand"]["likeEndpoint"];
    const QJsonValue toggledLikeEndpoint = toggledLikeViewModel.onTap["serialCommand"]["commands"][1]["innertubeCommand"]["likeEndpoint"];

    data.likeData.like = defaultLikeEndpoint["likeParams"].toString();
    data.likeData.removeLike = toggledLikeEndpoint["removeLikeParams"].toString();
    data.likeData.dislike = defaultLikeEndpoint["likeParams"].toString();
    data.likeData.removeDislike = toggledLikeEndpoint["removeLikeParams"].toString();

    if (g_settings->returnDislikes)
    {
        HttpReply* reply = HttpRequest().get("https://returnyoutubedislikeapi.com/votes?videoId=" + data.videoId);

        QEventLoop loop;
        QObject::connect(reply, &HttpReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->isSuccessful())
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            qint64 dislikes = doc["dislikes"].toVariant().toLongLong();
            qint64 likes = QLocale::system().toLongLong(fullLikeCount);
            data.likeDislikeRatio = double(likes) / (likes + dislikes);

        #ifdef QTTUBE_HAS_ICU
            if (g_settings->condensedCounts)
                data.dislikeCountText = StringUtils::condensedNumericString(dislikes);
            else
        #endif
                data.dislikeCountText = QLocale::system().toString(dislikes);
        }
        else
        {
            data.dislikeCountText = "Dislike";
        }
    }
    else
    {
        data.dislikeCountText = "Dislike";
    }

    data.dateText = primaryInfo.dateText.text;
    if (!primaryInfo.superTitleLink.text.isEmpty())
        data.dateText += " | " + primaryInfo.superTitleLink.toRichText(true);

    data.descriptionText = TubeUtils::unattribute(secondaryInfo.attributedDescription).toRichText(false);

    if (const QString& commentsContinuation = response.contents.results.commentsSectionContinuation; !commentsContinuation.isEmpty())
        data.continuations.comments = commentsContinuation;
    if (const QString& feedContinuation = response.contents.secondaryResults.feedContinuation; !feedContinuation.isEmpty())
        data.continuations.recommended = feedContinuation;
    for (const InnertubeObjects::WatchNextFeedItem& item : response.contents.secondaryResults.feed)
        std::visit([&data](auto&& v) { addVideo(data.recommendedVideos, v); }, item);
}

QtTube::NotificationsData getNotificationsData(const InnertubeEndpoints::NotificationMenuResponse& response)
{
    QtTube::NotificationsData result;
    for (const InnertubeObjects::Notification& notification : response.notifications)
        addNotification(result, convertNotification(notification));
    return result;
}

void getPlayerData(QtTube::VideoData& data, const InnertubeEndpoints::PlayerResponse& response)
{
    data.isLiveContent = response.videoDetails.isLive || response.videoDetails.isUpcoming;
    data.titleText = response.videoDetails.title;
}

QtTube::BrowseData getSearchData(const InnertubeEndpoints::SearchResponse& response)
{
    QtTube::BrowseData result;
    for (const InnertubeEndpoints::SearchResponseItem& item : response.contents)
    {
        if (const auto* channel = std::get_if<InnertubeObjects::Channel>(&item))
            addChannel(result, convertChannel(*channel));
        else if (const auto* rShelf = std::get_if<InnertubeObjects::ReelShelf>(&item); rShelf && !g_settings->hideSearchShelves)
            addShelf(result, convertShelf(*rShelf));
        else if (const auto* vShelf = std::get_if<InnertubeObjects::VerticalVideoShelf>(&item); vShelf && !g_settings->hideSearchShelves)
            addShelf(result, convertShelf(*vShelf));
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
            addVideo(result, *video);
    }
    return result;
}

QtTube::BrowseData getSubscriptionsData(const InnertubeEndpoints::SubscriptionsResponse& response)
{
    QtTube::BrowseData result;
    for (const InnertubeObjects::Video& video : response.videos)
        addVideo(result, video);
    return result;
}

QtTube::BrowseData getTrendingData(const InnertubeEndpoints::TrendingResponse& response)
{
    QtTube::BrowseData result;

    for (const InnertubeEndpoints::TrendingResponseItem& item : response.contents)
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
