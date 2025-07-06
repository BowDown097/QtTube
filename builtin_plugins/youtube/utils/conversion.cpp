#include "conversion.h"
#include "innertube/innertubereply.h"
#include <ranges>

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
    if (const auto* subscribeButton = std::get_if<InnertubeObjects::SubscribeButton>(&channel.subscribeButton))
        result.subscribeButton = convertSubscribeButton(*subscribeButton, *subscriberCountText);

    return result;
}

QtTube::PluginChannel convertChannel(
    const InnertubeObjects::VideoOwner& owner, const InnertubeObjects::SubscribeButton& subscribeButton)
{
    QtTube::PluginChannel result = {
        .channelId = owner.navigationEndpoint["browseEndpoint"]["browseId"].toString(),
        .channelName = owner.title.text,
        .channelUrlPrefix = "https://www.youtube.com/channel/",
        .sourceMetadata = &g_metadata
    };

    if (const InnertubeObjects::GenericThumbnail* recAvatar = owner.thumbnail.recommendedQuality(QSize(48, 48)))
        result.channelAvatarUrl = recAvatar->url;
    for (const InnertubeObjects::MetadataBadge& badge : owner.badges)
        result.channelBadges.append(convertBadge(badge));
    result.subscribeButton = convertSubscribeButton(subscribeButton, owner.subscriberCountText.text);

    return result;
}

QtTube::ChannelHeader convertChannelHeader(const InnertubeObjects::ChannelC4Header& header)
{
    QtTube::ChannelHeader result = {
        .channelSubtext = header.channelHandleText.text + " • " + header.videosCountText.text,
        .channelText = header.title,
        .subscribeButton = convertSubscribeButton(header.subscribeButton, header.subscriberCountText.text)
    };

    if (const InnertubeObjects::GenericThumbnail* recAvatar = header.avatar.recommendedQuality(QSize(48, 48)))
        result.avatarUrl = recAvatar->url;
    if (const InnertubeObjects::GenericThumbnail* bestBanner = header.banner.bestQuality())
        result.bannerUrl = bestBanner->url;

    return result;
}

QtTube::ChannelHeader convertChannelHeader(
    const InnertubeObjects::ChannelPageHeader& header, const QList<InnertubeObjects::EntityMutation>& mutations)
{
    QtTube::ChannelHeader result = {
        .channelText = header.title.text.content
    };

    if (const InnertubeObjects::GenericThumbnail* recAvatar = header.image.avatar.image.recommendedQuality(QSize(48, 48)))
        result.avatarUrl = recAvatar->url;
    if (const InnertubeObjects::GenericThumbnail* bestBanner = header.banner.image.bestQuality())
        result.bannerUrl = bestBanner->url;

    QString channelHandle = header.metadata.metadataRows.value(0).value(0).content;
    QString subCount = header.metadata.metadataRows.value(1).value(0).content;
    QString videosCount = header.metadata.metadataRows.value(1).value(1).content;
    result.channelSubtext = channelHandle + ' ' + header.metadata.delimiter + ' ' + videosCount;

    auto flatActions = header.actions.actionsRows
        | std::views::transform([](const auto& list) { return list.items; })
        | std::views::join;

    for (const auto& action : flatActions)
    {
        if (const auto* subscribeButton = std::get_if<InnertubeObjects::SubscribeButtonViewModel>(&action))
        {
            result.subscribeButton = convertSubscribeButton(*subscribeButton, subCount, subscribeButton->isSubscribed(mutations));
        }
        else if (const auto* plainButton = std::get_if<InnertubeObjects::ButtonViewModel>(&action))
        {
            // logged out subscribe button should be the only one with a modalEndpoint
            if (plainButton->onTap["innertubeCommand"]["modalEndpoint"].isObject())
                result.subscribeButton = convertSubscribeButton(*plainButton, subCount);
        }
    }

    return result;
}

QtTube::PluginException convertException(const InnertubeException& ex)
{
    return QtTube::PluginException(ex.message(), static_cast<QtTube::PluginException::Severity>(ex.severity()));
}

QtTube::LiveChatItem convertLiveChatItem(const QJsonValue& item)
{
    if (const QJsonValue textMessage = item["liveChatTextMessageRenderer"]; textMessage.isObject()) [[likely]]
    {
        QtTube::TextMessage result = {
            .authorName = InnertubeObjects::InnertubeString(textMessage["authorName"]).text,
            .content = InnertubeObjects::InnertubeString(textMessage["message"]).toRichText(false),
        };

        InnertubeObjects::ResponsiveImage authorPhoto(textMessage["authorPhoto"]["thumbnails"]);
        if (const InnertubeObjects::GenericThumbnail* recPhoto = authorPhoto.recommendedQuality(QSize(32, 32)))
            result.authorAvatarUrl = recPhoto->url;

        if (const QJsonArray authorBadges = textMessage["authorBadges"].toArray(); !authorBadges.isEmpty())
        {
            bool isModerator = std::ranges::any_of(authorBadges, [](const QJsonValue& badge) {
                return badge["liveChatAuthorBadgeRenderer"]["icon"]["iconType"].toString() == "MODERATOR";
            });

            // if not moderator, assume member (is there anything else?)
            result.authorNameColor = isModerator ? "#5e84f1" : "#2ba640";
        }

        if (const QString timestampText = textMessage["timestampText"].toString(); !timestampText.isEmpty())
        {
            result.timestampText = timestampText;
        }
        else
        {
            quint64 timestampUsec = textMessage["timestampUsec"].toString().toULongLong();
            result.timestampText = QDateTime::fromSecsSinceEpoch(timestampUsec / 1000000)
                .toString(QLocale::system().timeFormat(QLocale::ShortFormat));
        }

        return result;
    }
    else if (const QJsonValue membershipItem = item["liveChatMembershipItemRenderer"]; membershipItem.isObject())
    {
        return QtTube::SpecialMessage {
            .backgroundColor = "#0f9d58",
            .content = InnertubeObjects::InnertubeString(membershipItem["headerSubtext"]).text,
            .header = InnertubeObjects::InnertubeString(membershipItem["authorName"]).text
        };
    }
    else if (const QJsonValue modeChangeMessage = item["liveChatModeChangeMessageRenderer"]; modeChangeMessage.isObject())
    {
        return QtTube::SpecialMessage {
            .backgroundColor = "black",
            .content = InnertubeObjects::InnertubeString(modeChangeMessage["subtext"]).text,
            .contentStyle = QFont::StyleItalic,
            .header = InnertubeObjects::InnertubeString(modeChangeMessage["text"]).text
        };
    }
    else if (const QJsonValue paidMessage = item["liveChatPaidMessageRenderer"]; paidMessage.isObject())
    {
        QtTube::PaidMessage result = {
            .authorName = InnertubeObjects::InnertubeString(paidMessage["authorName"]).text,
            .content = InnertubeObjects::InnertubeString(paidMessage["message"]).toRichText(false),
            .contentBackgroundColor = '#' + QString::number(paidMessage["bodyBackgroundColor"].toInteger(), 16),
            .contentTextColor = '#' + QString::number(paidMessage["bodyTextColor"].toInteger(), 16),
            .headerBackgroundColor = '#' + QString::number(paidMessage["headerBackgroundColor"].toInteger(), 16),
            .headerTextColor = '#' + QString::number(paidMessage["headerTextColor"].toInteger(), 16),
            .paidAmountText = InnertubeObjects::InnertubeString(paidMessage["purchaseAmountText"]).text
        };

        InnertubeObjects::ResponsiveImage authorPhoto(paidMessage["authorPhoto"]["thumbnails"]);
        if (const InnertubeObjects::GenericThumbnail* recPhoto = authorPhoto.recommendedQuality(QSize(32, 32)))
            result.authorAvatarUrl = recPhoto->url;

        return result;
    }
    else if (const QJsonValue giftMessage = item["liveChatSponsorshipsGiftRedemptionAnnouncementRenderer"]; giftMessage.isObject())
    {
        return QtTube::GiftRedemptionMessage {
            .authorName = InnertubeObjects::InnertubeString(giftMessage["authorName"]).text,
            .content = InnertubeObjects::InnertubeString(giftMessage["message"]).text
        };
    }
    else if (const QJsonValue engagement = item["liveChatViewerEngagementMessageRenderer"]; engagement.isObject())
    {
        return QtTube::SpecialMessage {
            .backgroundColor = "black",
            .content = InnertubeObjects::InnertubeString(engagement["message"]).text,
            .header = InnertubeObjects::InnertubeString(engagement["text"]).text
        };
    }

    return {};
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

QtTube::PluginSubscribeButton convertSubscribeButton(
    const InnertubeObjects::ButtonViewModel& button, const QString& countText)
{
    return QtTube::PluginSubscribeButton {
        .countText = countText.left(countText.lastIndexOf(' ')),
        .enabled = false,
        .localization = {
            .subscribeText = button.title
        }
    };
}

QtTube::PluginSubscribeButton convertSubscribeButton(
    const InnertubeObjects::SubscribeButton& subscribeButton, const QString& countText)
{
    const QJsonValue unsubscribeDialog = subscribeButton.onUnsubscribeEndpoints
        [0]["signalServiceEndpoint"]["actions"][0]["openPopupAction"]["popup"]["confirmDialogRenderer"];

    QtTube::PluginSubscribeButton result = {
        .countText = countText.left(countText.lastIndexOf(' ')),
        .enabled = subscribeButton.enabled,
        .localization = {
            .subscribeText = subscribeButton.unsubscribedButtonText.text,
            .subscribedText = subscribeButton.subscribedButtonText.text,
            .unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text,
            .unsubscribeText = subscribeButton.unsubscribeButtonText.text
        },
        .subscribed = subscribeButton.subscribed,
        .subscribeData = subscribeButton.onSubscribeEndpoints[0]["subscribeEndpoint"],
        .unsubscribeData = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"]
    };

    const InnertubeObjects::NotificationPreferenceButton& notifButton = subscribeButton.notificationPreferenceButton;

    for (qsizetype i = 0; i < notifButton.popup.items.size(); ++i)
    {
        const InnertubeObjects::MenuServiceItem& item = notifButton.popup.items[i];
        if (item.isSelected && result.notificationBell.defaultEnabledStateIndex == -1)
            result.notificationBell.activeStateIndex = i;

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
            result.notificationBell.defaultEnabledStateIndex = i;
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

        result.notificationBell.states.append(state);
    }

    return result;
}

QtTube::PluginSubscribeButton convertSubscribeButton(
    const InnertubeObjects::SubscribeButtonViewModel& subscribeButton, const QString& countText, bool subscribed)
{
    const QJsonValue unsubscribeDialog = subscribeButton.unsubscribeButtonContent.onTapCommand
        ["innertubeCommand"]["signalServiceEndpoint"]["actions"][0]["openPopupAction"]["popup"]["confirmDialogRenderer"];

    QtTube::PluginSubscribeButton result = {
        .countText = countText.left(countText.lastIndexOf(' ')),
        .enabled = !subscribeButton.disableSubscribeButton,
        .localization = {
            .subscribeText = subscribeButton.subscribeButtonContent.buttonText,
            .subscribedText = subscribeButton.unsubscribeButtonContent.buttonText,
            .unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text,
            .unsubscribeText = "Unsubscribe"
        },
        .subscribed = subscribed,
        .subscribeData = subscribeButton.subscribeButtonContent.onTapCommand["innertubeCommand"]["subscribeEndpoint"],
        .unsubscribeData = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"]
    };

    if (subscribeButton.disableNotificationBell)
        return result;

    const QJsonArray listItems = subscribeButton.onShowSubscriptionOptions
        ["innertubeCommand"]["showSheetCommand"]["panelLoadingStrategy"]
        ["inlineContent"]["sheetViewModel"]["content"]["listViewModel"]
        ["listItems"].toArray();

    for (qsizetype i = 0; i < listItems.size(); ++i)
    {
        const QJsonValue viewModel = listItems[i]["listItemViewModel"];
        if (viewModel["isSelected"].toBool() && result.notificationBell.defaultEnabledStateIndex == -1)
            result.notificationBell.activeStateIndex = i;

        QtTube::PluginNotificationState state = {
            .data = viewModel["rendererContext"]["commandContext"]["onTap"]["innertubeCommand"]
                ["modifyChannelNotificationPreferenceEndpoint"]["params"].toString(),
            .name = viewModel["title"]["content"].toString()
        };

        QString imageName = viewModel["leadingImage"]["sources"][0]["clientResource"]["imageName"].toString();
        if (imageName == "NOTIFICATIONS_ACTIVE")
        {
            state.representation = QtTube::PluginNotificationState::Representation::All;
        }
        else if (imageName == "NOTIFICATIONS_NONE")
        {
            result.notificationBell.defaultEnabledStateIndex = i;
            state.representation = QtTube::PluginNotificationState::Representation::Neutral;
        }
        else if (imageName == "NOTIFICATIONS_OFF")
        {
            state.representation = QtTube::PluginNotificationState::Representation::None;
        }
        else
        {
            continue;
        }

        result.notificationBell.states.append(state);
    }

    return result;
}

void processRichGrid(const QJsonValue& richGrid, QList<QtTube::ChannelTabDataItem>& items, std::any& continuationData)
{
    const QJsonArray contents = richGrid["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        const QJsonValue itemContent = v["richItemRenderer"]["content"];
        if (const QJsonValue video = itemContent["videoRenderer"]; video.isObject())
            items.append(convertVideo(InnertubeObjects::Video(video), true));
        else if (const QJsonValue reel = itemContent["reelItemRenderer"]; reel.isObject())
            items.append(convertVideo(InnertubeObjects::Reel(video), true));
        else if (const QJsonValue shortsLockup = itemContent["shortsLockupViewModel"]; shortsLockup.isObject())
            items.append(convertVideo(InnertubeObjects::ShortsLockupViewModel(shortsLockup), true));
        else if (const QJsonValue continuation = v["continuationItemRenderer"]; continuation.isObject())
            continuationData = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
    }
}

void processSectionList(const QJsonValue& sectionList, QList<QtTube::ChannelTabDataItem>& items, std::any& continuationData)
{
    const QJsonArray contents = sectionList["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        const QJsonArray itemSectionContents = v["itemSectionRenderer"]["contents"].toArray();
        for (const QJsonValue& v2 : itemSectionContents)
        {
            if (const QJsonValue shelf = v2["shelfRenderer"]; shelf.isObject())
            {
                const QJsonObject shelfContent = shelf["content"].toObject();
                const QJsonArray shelfItems = ((const QJsonValue&)(*shelfContent.begin()))["items"].toArray();
                if (shelfItems.isEmpty())
                    continue;

                const QString shelfKey = shelfItems.begin()->toObject().begin().key();
                if (shelfKey == "channelRenderer" || shelfKey == "gridChannelRenderer")
                {
                    QtTube::PluginShelf<QtTube::PluginChannel> channelShelf;
                    channelShelf.title = InnertubeObjects::InnertubeString(shelf["title"]).text;
                    for (const QJsonValue& v3 : shelfItems)
                        channelShelf.contents.append(convertChannel(InnertubeObjects::Channel(v3[shelfKey])));
                    items.append(channelShelf);
                }
                else if (shelfKey == "gridVideoRenderer" || shelfKey == "videoRenderer")
                {
                    QtTube::PluginShelf<QtTube::PluginVideo> videoShelf;
                    videoShelf.title = InnertubeObjects::InnertubeString(shelf["title"]).text;
                    for (const QJsonValue& v3 : shelfItems)
                        videoShelf.contents.append(convertVideo(InnertubeObjects::Video(v3[shelfKey]), true));
                    items.append(videoShelf);
                }
            }
            else if (const QJsonValue continuation = v2["continuationItemRenderer"]; continuation.isObject())
            {
                continuationData = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            }
        }
    }
}

QtTube::ChannelTabData convertTab(const QJsonValue& tabRenderer, std::any& continuationData)
{
    QtTube::ChannelTabData result = {
        .requestData = tabRenderer["endpoint"]["browseEndpoint"]["params"].toString(),
        .title = tabRenderer["title"].toString()
    };

    if (const QJsonValue tabContent = tabRenderer["content"]; tabContent.isObject())
    {
        if (const QJsonValue richGrid = tabContent["richGridRenderer"]; richGrid.isObject())
            processRichGrid(richGrid, result.items, continuationData);
        else if (const QJsonValue sectionList = tabContent["sectionListRenderer"]; sectionList.isObject())
            processSectionList(sectionList, result.items, continuationData);
    }

    return result;
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
        .isVerticalVideo = true,
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
        .isVerticalVideo = true,
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
