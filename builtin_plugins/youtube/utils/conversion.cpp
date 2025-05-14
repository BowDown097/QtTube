#include "conversion.h"
#include "youtubeplugin.h"

QtTube::PluginVideoBadge convertBadge(const InnertubeObjects::MetadataBadge& badge)
{
    return QtTube::PluginVideoBadge {
        .tooltip = badge.tooltip
    };
}

QtTube::PluginException convertException(const InnertubeException& ex)
{
    return QtTube::PluginException(ex.message(), static_cast<QtTube::PluginException::Severity>(ex.severity()));
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::CompactVideo& compactVideo, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = compactVideo.lengthDisplay().text,
        .progressSecs = compactVideo.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt(),
        .sourceIconUrl = g_metadata.image,
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
        if (const InnertubeObjects::GenericThumbnail* recAvatar = owner.icon.recommendedQuality(QSize(200, 200)))
            result.uploaderAvatarUrl = recAvatar->url;
        for (const InnertubeObjects::MetadataBadge& badge : compactVideo.ownerBadges)
            result.uploaderBadges.append(convertBadge(badge));
    }

    result.thumbnailUrl = useThumbnailFromData && !compactVideo.thumbnail.isEmpty()
        ? compactVideo.thumbnail.recommendedQuality(QSize(200, 200))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::DisplayAd& displayAd, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = "Ad",
        .metadataText = displayAd.bodyText.text,
        .sourceIconUrl = g_metadata.image,
        .thumbnailUrl = displayAd.image.recommendedQuality(QSize(200, 200))->url,
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
        .sourceIconUrl = g_metadata.image,
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
        if (const InnertubeObjects::GenericThumbnail* recAvatar = owner->icon.recommendedQuality(QSize(200, 200)))
            result.uploaderAvatarUrl = recAvatar->url;
    }

    result.thumbnailUrl = useThumbnailFromData && !lockup.contentImage.image.isEmpty()
        ? lockup.contentImage.image.recommendedQuality(QSize(200, 200))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::Reel& reel, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = "SHORTS",
        .metadataText = reel.viewCountText.text,
        .sourceIconUrl = g_metadata.image,
        .title = QString(reel.headline).replace("\r\n", " "),
        .videoId = reel.videoId,
        .videoUrlPrefix = "https://www.youtube.com/shorts/"
    };

    result.thumbnailUrl = useThumbnailFromData && !reel.thumbnail.isEmpty()
        ? reel.thumbnail.recommendedQuality(QSize(200, 200))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::ShortsLockupViewModel& shortsLockup, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = "SHORTS",
        .metadataText = shortsLockup.secondaryText,
        .sourceIconUrl = g_metadata.image,
        .title = QString(shortsLockup.primaryText).replace("\r\n", " "),
        .videoId = shortsLockup.videoId,
        .videoUrlPrefix = "https://www.youtube.com/shorts/"
    };

    result.thumbnailUrl = useThumbnailFromData && !shortsLockup.thumbnail.isEmpty()
        ? shortsLockup.thumbnail.recommendedQuality(QSize(200, 200))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::Video& video, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .lengthText = video.lengthDisplay().text,
        .progressSecs = video.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt(),
        .sourceIconUrl = g_metadata.image,
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
        if (const InnertubeObjects::GenericThumbnail* recAvatar = video.channelThumbnailSupportedRenderers.thumbnail.recommendedQuality(QSize(200, 200)))
            result.uploaderAvatarUrl = recAvatar->url;
        for (const InnertubeObjects::MetadataBadge& badge : video.ownerBadges)
            result.uploaderBadges.append(convertBadge(badge));
    }

    result.thumbnailUrl = useThumbnailFromData && !video.thumbnail.isEmpty()
        ? video.thumbnail.recommendedQuality(QSize(200, 200))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}

QtTube::PluginVideo convertVideo(const InnertubeObjects::VideoDisplayButtonGroup& video, bool useThumbnailFromData)
{
    QtTube::PluginVideo result = {
        .metadataText = video.badge.label,
        .sourceIconUrl = g_metadata.image,
        .title = QString(video.title.text).replace("\r\n", " "),
        .videoId = video.videoId,
        .videoUrlPrefix = "https://www.youtube.com/watch?v="
    };

    if (video.channelEndpoint.isObject())
    {
        if (const QJsonValue urlEndpoint = video.channelEndpoint["urlEndpoint"]; urlEndpoint.isObject())
            result.uploaderId = urlEndpoint["url"].toString();
        if (const InnertubeObjects::GenericThumbnail* recAvatar = video.channelThumbnail.recommendedQuality(QSize(200, 200)))
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
        ? video.thumbnail.recommendedQuality(QSize(200, 200))->url
        : "https://img.youtube.com/vi/" + result.videoId + "/mqdefault.jpg";

    return result;
}
