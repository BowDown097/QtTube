#include "conversion.h"
#include "innertube/innertubereply.h"

QtTube::PluginBadge convertBadge(const InnertubeObjects::MetadataBadge& badge)
{
    return QtTube::PluginBadge {
        .label = badge.style == "BADGE_STYLE_TYPE_VERIFIED_ARTIST" ? "♪" : "✔",
        .tooltip = badge.tooltip
    };
}

QtTube::PluginChannel convertChannel(const InnertubeObjects::Channel& channel)
{
    QtTube::PluginChannel result = {
        .channelId = channel.channelId,
        .channelName = channel.title.text,
        .channelUrlPrefix = "https://www.youtube.com/channel/",
        .description = channel.descriptionSnippet.text,
        .sourceMetadata = &g_metadata
    };

    if (const InnertubeObjects::GenericThumbnail* recAvatar = channel.thumbnail.recommendedQuality(QSize(80, 80)))
        result.channelAvatarUrl = "https:" + recAvatar->url;
    for (const InnertubeObjects::MetadataBadge& badge : channel.ownerBadges)
        result.channelBadges.append(convertBadge(badge));

    // "google lied to you!" - kanye west
    // subscriberCountText and videoCountText may be what they say, but they also may not.
    // either one can actually be the channel handle (but never both), so we have to check for that.
    const QString* subscriberCountText = &channel.subscriberCountText.text;
    const QString* metadataText = &channel.videoCountText.text;
    if (subscriberCountText->startsWith('@'))
        std::swap(subscriberCountText, metadataText);

    result.metadataText = *metadataText;
    result.subscribeButton.countText = subscriberCountText->left(subscriberCountText->lastIndexOf(' '));

    result.subscribeButton.subscribeText = "Subscribe";
    result.subscribeButton.subscribedText = "Subscribed";

    if (const auto* subscribeButton = std::get_if<InnertubeObjects::SubscribeButton>(&channel.subscribeButton))
    {
        result.subscribeButton.enabled = subscribeButton->enabled;
        result.subscribeButton.subscribed = subscribeButton->subscribed;
        result.subscribeButton.subscribeData = subscribeButton->onSubscribeEndpoints[0]["subscribeEndpoint"];

        result.subscribeButton.subscribeText = subscribeButton->unsubscribedButtonText.text;
        result.subscribeButton.subscribedText = subscribeButton->subscribedButtonText.text;
        result.subscribeButton.unsubscribeText = subscribeButton->unsubscribeButtonText.text;

        const QJsonValue unsubscribeDialog = subscribeButton->onUnsubscribeEndpoints
            [0]["signalServiceEndpoint"]["actions"][0]["openPopupAction"]["popup"]["confirmDialogRenderer"];
        result.subscribeButton.unsubscribeData =
            unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"];
        result.subscribeButton.unsubscribeDialogText =
            InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text;

        QtTube::PluginNotificationBell& notifBell = result.subscribeButton.notificationBell;
        const InnertubeObjects::NotificationPreferenceButton& notifButton = subscribeButton->notificationPreferenceButton;

        for (qsizetype i = 0; i < notifButton.popup.items.size(); ++i)
        {
            const InnertubeObjects::MenuServiceItem& item = notifButton.popup.items[i];
            if (item.isSelected && notifBell.defaultEnabledStateIndex == -1)
                notifBell.activeStateIndex = i;

            QtTube::PluginNotificationState state = {
                .data = item.serviceEndpoint["modifyChannelNotificationPreferenceEndpoint"]["params"].toString(),
                .name = item.text.text
            };

            if (item.iconType == "NOTIFICATIONS_ACTIVE")
            {
                state.representation = QtTube::PluginNotificationState::Representation::All;
            }
            else if (item.iconType == "NOTIFICATIONS_NONE")
            {
                notifBell.defaultEnabledStateIndex = i;
                state.representation = QtTube::PluginNotificationState::Representation::Neutral;
            }
            else if (item.iconType == "NOTIFICATIONS_OFF")
            {
                state.representation = QtTube::PluginNotificationState::Representation::None;
            }
            else
            {
                continue;
            }

            notifBell.states.append(state);
        }
    }

    return result;
}

QtTube::PluginException convertException(const InnertubeException& ex)
{
    return QtTube::PluginException(ex.message(), static_cast<QtTube::PluginException::Severity>(ex.severity()));
}

QtTube::PluginNotification convertNotification(const InnertubeObjects::Notification& notification)
{
    QtTube::PluginNotification result = {
        .body = notification.shortMessage,
        .notificationId = notification.notificationId,
        .sentTimeText = notification.sentTimeText,
        .sourceMetadata = &g_metadata
    };

    if (const InnertubeObjects::GenericThumbnail* channelThumb = notification.channelIcon.recommendedQuality(QSize(48, 48)))
        result.channelAvatarUrl = channelThumb->url;

    // TODO: see if there's other endpoints (there probably is)
    if (const QJsonValue getCommentsFromInboxCommand = notification.navigationEndpoint["getCommentsFromInboxCommand"];
        getCommentsFromInboxCommand.isObject())
    {
        // may extend this later, hence why it's its own block
        result.targetId = getCommentsFromInboxCommand["videoId"].toString();
        result.targetType = QtTube::PluginNotification::TargetType::Video;
        result.targetUrlPrefix = "https://www.youtube.com/watch?v=";

        // notification.videoThumbnail returns images with black bars, so we're going to use mqdefault instead
        result.thumbnailUrl = "https://i.ytimg.com/vi/" + result.targetId + "/mqdefault.jpg";
    }
    else if (const QJsonValue watchEndpoint = notification.navigationEndpoint["watchEndpoint"];
             watchEndpoint.isObject())
    {
        result.targetId = watchEndpoint["videoId"].toString();
        result.targetType = QtTube::PluginNotification::TargetType::Video;
        result.targetUrlPrefix = "https://www.youtube.com/watch?v=";

        // notification.videoThumbnail returns images with black bars, so we're going to use mqdefault instead
        result.thumbnailUrl = "https://i.ytimg.com/vi/" + result.targetId + "/mqdefault.jpg";
    }

    return result;
}

QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(
    const InnertubeObjects::HomeRichShelf& hrShelf, bool useThumbnailFromData)
{
    QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
    videoShelf.isDividerHidden = hrShelf.isBottomDividerHidden;
    videoShelf.subtitle = hrShelf.subtitle.text;
    videoShelf.title = hrShelf.title.text;
    if (const InnertubeObjects::GenericThumbnail* bestThumb = hrShelf.thumbnail.bestQuality())
        videoShelf.iconUrl = bestThumb->url;

    for (const auto& itemVariant : hrShelf.contents)
    {
        std::visit([&videoShelf, useThumbnailFromData](auto&& item) {
            using ItemType = std::remove_cvref_t<decltype(item)>;
            if constexpr (!innertube_is_any_v<ItemType, InnertubeObjects::MiniGameCardViewModel, InnertubeObjects::Post>)
                addVideo(videoShelf.contents, item, useThumbnailFromData);
        }, itemVariant);
    }

    return videoShelf;
}

QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(
    const InnertubeObjects::HorizontalVideoShelf& hShelf, bool useThumbnailFromData)
{
    QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
    videoShelf.title = hShelf.title.text;
    if (const InnertubeObjects::GenericThumbnail* bestThumb = hShelf.thumbnail.bestQuality())
        videoShelf.iconUrl = bestThumb->url;

    for (const InnertubeObjects::Video& video : hShelf.content.items)
        addVideo(videoShelf.contents, video, useThumbnailFromData);

    return videoShelf;
}

QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::ReelShelf& rShelf)
{
    QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
    videoShelf.title = rShelf.title.text;
    for (const InnertubeObjects::ShortsLockupViewModel& reel : rShelf.items)
        addVideo(videoShelf.contents, reel);
    return videoShelf;
}

QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::StandardVideoShelf& sShelf)
{
    QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
    videoShelf.title = sShelf.title.text;
    if (const InnertubeObjects::GenericThumbnail* bestThumb = sShelf.thumbnail.bestQuality())
        videoShelf.iconUrl = bestThumb->url;

    for (const InnertubeObjects::Video& video : sShelf.content)
        addVideo(videoShelf.contents, video);

    return videoShelf;
}

QtTube::PluginShelf<QtTube::PluginVideo> convertShelf(const InnertubeObjects::VerticalVideoShelf& vShelf)
{
    QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
    videoShelf.title = vShelf.title.text;
    if (const InnertubeObjects::GenericThumbnail* bestThumb = vShelf.thumbnail.bestQuality())
        videoShelf.iconUrl = bestThumb->url;

    for (const InnertubeObjects::Video& video : vShelf.content.items)
        addVideo(videoShelf.contents, video);

    return videoShelf;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::CompactVideo& compactVideo, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = compactVideo.lengthDisplay().text,
        .progressSecs = compactVideo.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt(),
        .sourceMetadata = &g_metadata,
        .title = QString(compactVideo.title.text).replace("\r\n", " "),
        .videoId = compactVideo.videoId,
        .videoUrlPrefix = "https://www.youtube.com/watch?v="
    };

    QStringList metadataList;
    metadataList.reserve(2);
    metadataList.append(g_settings->condensedCounts ? compactVideo.shortViewCountText.text : compactVideo.viewCountText.text);
    metadataList.append(compactVideo.publishedTimeText.text);
    metadataList.removeAll({});
    result.metadataText = metadataList.join(" • ");

    if (InnertubeObjects::BasicChannel owner = compactVideo.owner(); !owner.id.isEmpty())
    {
        result.uploaderId = owner.id;
        result.uploaderText = owner.name;
        result.uploaderUrlPrefix = "https://www.youtube.com/channel/";
        if (const InnertubeObjects::GenericThumbnail* recAvatar = owner.icon.recommendedQuality(QSize(205, 205)))
            result.uploaderAvatarUrl = recAvatar->url;
        for (const InnertubeObjects::MetadataBadge& badge : compactVideo.ownerBadges)
            result.uploaderBadges.append(convertBadge(badge));
    }

    result.thumbnailUrl = useThumbnailFromData && !compactVideo.thumbnail.isEmpty()
        ? compactVideo.thumbnail.recommendedQuality(QSize(205, 205))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::DisplayAd& displayAd, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = "Ad",
        .metadataText = displayAd.bodyText.text,
        .sourceMetadata = &g_metadata,
        .thumbnailUrl = displayAd.image.recommendedQuality(QSize(205, 205))->url,
        .title = QString(displayAd.titleText.text).replace("\r\n", " ")
    };

    if (displayAd.clickCommand.isObject())
    {
        if (const QJsonValue urlEndpoint = displayAd.clickCommand["urlEndpoint"]; urlEndpoint.isObject())
            result.uploaderId = result.videoId = urlEndpoint["url"].toString();
        result.uploaderText = displayAd.secondaryText.text;
    }

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::LockupViewModel& lockup, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = lockup.lengthText(),
        .progressSecs = lockup.rendererContext["commandContext"]["onTap"]["innertubeCommand"]["watchEndpoint"]["startTimeSeconds"].toInt(),
        .sourceMetadata = &g_metadata,
        .title = QString(lockup.metadata.title).replace("\r\n", " "),
        .videoId = lockup.contentId,
        .videoUrlPrefix = "https://www.youtube.com/watch?v="
    };

    if (lockup.metadata.metadata.metadataRows.size() > 1)
    {
        QStringList metadataList;
        for (const InnertubeObjects::DynamicText& part : lockup.metadata.metadata.metadataRows[1])
            metadataList.append(part.content);
        result.metadataText = metadataList.join(lockup.metadata.metadata.delimiter);
    }

    if (std::optional<InnertubeObjects::BasicChannel> owner = lockup.owner())
    {
        result.uploaderId = owner->id;
        result.uploaderText = owner->name;
        result.uploaderUrlPrefix = "https://www.youtube.com/channel/";
        if (const InnertubeObjects::GenericThumbnail* recAvatar = owner->icon.recommendedQuality(QSize(205, 205)))
            result.uploaderAvatarUrl = recAvatar->url;
    }

    result.thumbnailUrl = useThumbnailFromData && !lockup.contentImage.image.isEmpty()
        ? lockup.contentImage.image.recommendedQuality(QSize(205, 205))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::Reel& reel, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = "SHORTS",
        .metadataText = reel.viewCountText.text,
        .sourceMetadata = &g_metadata,
        .title = QString(reel.headline).replace("\r\n", " "),
        .videoId = reel.videoId,
        .videoUrlPrefix = "https://www.youtube.com/shorts/"
    };

    result.thumbnailUrl = useThumbnailFromData && !reel.thumbnail.isEmpty()
        ? reel.thumbnail.recommendedQuality(QSize(205, 205))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::ShortsLockupViewModel& shortsLockup, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = "SHORTS",
        .metadataText = shortsLockup.secondaryText,
        .sourceMetadata = &g_metadata,
        .title = QString(shortsLockup.primaryText).replace("\r\n", " "),
        .videoId = shortsLockup.videoId,
        .videoUrlPrefix = "https://www.youtube.com/shorts/"
    };

    result.thumbnailUrl = useThumbnailFromData && !shortsLockup.thumbnail.isEmpty()
        ? shortsLockup.thumbnail.recommendedQuality(QSize(205, 205))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::Video& video, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = video.lengthDisplay().text,
        .progressSecs = video.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt(),
        .sourceMetadata = &g_metadata,
        .title = QString(video.title.text).replace("\r\n", " "),
        .videoId = video.videoId,
        .videoUrlPrefix = "https://www.youtube.com/watch?v="
    };

    QStringList metadataList;
    metadataList.reserve(2);
    metadataList.append(g_settings->condensedCounts ? video.shortViewCountText.text : video.viewCountText.text);
    metadataList.append(video.publishedTimeDisplay());
    metadataList.removeAll({});
    result.metadataText = metadataList.join(" • ");

    if (QString ownerId = video.ownerId(); !ownerId.isEmpty())
    {
        result.uploaderId = ownerId;
        result.uploaderText = video.ownerText.text;
        result.uploaderUrlPrefix = "https://www.youtube.com/channel/";
        if (const InnertubeObjects::GenericThumbnail* recAvatar = video.channelThumbnailSupportedRenderers.thumbnail.recommendedQuality(QSize(205, 205)))
            result.uploaderAvatarUrl = recAvatar->url;
        for (const InnertubeObjects::MetadataBadge& badge : video.ownerBadges)
            result.uploaderBadges.append(convertBadge(badge));
    }

    result.thumbnailUrl = useThumbnailFromData && !video.thumbnail.isEmpty()
        ? video.thumbnail.recommendedQuality(QSize(205, 205))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::VideoDisplayButtonGroup& video, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .metadataText = video.badge.label,
        .sourceMetadata = &g_metadata,
        .title = QString(video.title.text).replace("\r\n", " "),
        .videoId = video.videoId,
        .videoUrlPrefix = "https://www.youtube.com/watch?v="
    };

    if (video.channelEndpoint.isObject())
    {
        if (const QJsonValue urlEndpoint = video.channelEndpoint["urlEndpoint"]; urlEndpoint.isObject())
            result.uploaderId = urlEndpoint["url"].toString();
        if (const InnertubeObjects::GenericThumbnail* recAvatar = video.channelThumbnail.recommendedQuality(QSize(205, 205)))
            result.uploaderAvatarUrl = recAvatar->url;
        result.uploaderText = video.shortBylineText.text;
    }

    auto it = std::ranges::find_if(video.thumbnailOverlays, [](const InnertubeObjects::ThumbnailOverlay& overlay) {
        return std::holds_alternative<InnertubeObjects::ThumbnailOverlayTimeStatus>(overlay);
    });

    if (it != video.thumbnailOverlays.end())
    {
        const auto& timeOverlay = std::get<InnertubeObjects::ThumbnailOverlayTimeStatus>(*it);
        result.lengthText = timeOverlay.iconType != "EXTERNAL_LINK" ? timeOverlay.text.text : "Ad";
    }
    else
    {
        result.lengthText = video.lengthText.text;
    }

    result.thumbnailUrl = useThumbnailFromData && !video.thumbnail.isEmpty()
        ? video.thumbnail.recommendedQuality(QSize(205, 205))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}
