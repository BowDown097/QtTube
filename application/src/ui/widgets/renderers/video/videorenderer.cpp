#include "videorenderer.h"
#include "http.h"
#include "innertube.h"
#include "innertube/objects/video/reel.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include "ui/views/preloaddata.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "videothumbnailwidget.h"
#include <QDesktopServices>
#include <QJsonDocument>
#include <QMenu>
#include <QMessageBox>

VideoRenderer::VideoRenderer(QWidget* parent)
    : QWidget(parent),
      channelLabel(new ChannelLabel(this)),
      metadataLabel(new TubeLabel(this)),
      thumbnail(new VideoThumbnailWidget(this)),
      titleLabel(new TubeLabel(this))
{
    channelLabel->addStretch();
    channelLabel->hide();

    titleLabel->setClickable(true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setElideMode(Qt::ElideRight);
    titleLabel->setFont(QFont(font().toString(), font().pointSize() + 2, QFont::Bold));
    titleLabel->setUnderlineOnHover(true);

    connect(thumbnail, &VideoThumbnailWidget::clicked, this, &VideoRenderer::navigate);
    connect(titleLabel, &TubeLabel::clicked, this, &VideoRenderer::navigate);
    connect(titleLabel, &TubeLabel::customContextMenuRequested, this, &VideoRenderer::showTitleContextMenu);
}

void VideoRenderer::copyDirectUrl()
{
    auto reply = InnerTube::instance()->get<InnertubeEndpoints::Player>(videoId);
    connect(reply, &InnertubeReply<InnertubeEndpoints::Player>::exception, this, [this] {
        QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely unavailable.");
    });
    connect(reply, &InnertubeReply<InnertubeEndpoints::Player>::finished, this, [this](const InnertubeEndpoints::Player& endpoint) {
        const InnertubeObjects::StreamingData& streamingData = endpoint.response.streamingData;
        const InnertubeObjects::PlayerVideoDetails videoDetails = endpoint.response.videoDetails;
        if (videoDetails.isLive || videoDetails.isLiveContent)
        {
            UIUtils::copyToClipboard(streamingData.hlsManifestUrl);
        }
        else
        {
            if (auto best = std::ranges::max_element(
                    streamingData.formats, std::less(), &InnertubeObjects::StreamingFormat::bitrate);
                best != streamingData.formats.end())
            {
                UIUtils::copyToClipboard(best->url);
            }
            else
            {
                QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely unavailable.");
            }
        }
    });
}

void VideoRenderer::copyVideoUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/watch?v=" + videoId);
}

void VideoRenderer::navigate()
{
    if (!videoId.isEmpty())
        ViewController::loadVideo(videoId, progress, watchPreloadData.get());
    else if (const QJsonValue urlEndpoint = videoEndpoint["urlEndpoint"]; urlEndpoint.isObject())
        QDesktopServices::openUrl(urlEndpoint["url"].toString());
}

void VideoRenderer::setData(const InnertubeObjects::CompactVideo& compactVideo,
                            bool useThumbnailFromData)
{
    progress = compactVideo.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt();
    videoId = compactVideo.videoId;
    watchPreloadData = std::make_unique<PreloadData::WatchView>();

    QStringList metadataList;
    metadataList.reserve(2);
    metadataList.append(qtTubeApp->settings().condensedCounts
        ? compactVideo.shortViewCountText.text : compactVideo.viewCountText.text);
    metadataList.append(compactVideo.publishedTimeText.text);
    metadataList.removeAll({});
    metadataLabel->setText(metadataList.join(" • "));

    if (InnertubeObjects::BasicChannel owner = compactVideo.owner(); !owner.id.isEmpty())
    {
        watchPreloadData->channelAvatar = owner.icon;
        watchPreloadData->channelBadges = compactVideo.ownerBadges;
        watchPreloadData->channelId = owner.id;
        watchPreloadData->channelName = owner.name;

        channelLabel->show();
        channelLabel->setInfo(owner.id, owner.name, compactVideo.ownerBadges);
    }

    thumbnail->setLengthText(compactVideo.lengthDisplay().text);
    thumbnail->setProgress(progress, QTime(0, 0).secsTo(compactVideo.length()));

    if (useThumbnailFromData && !compactVideo.thumbnail.isEmpty())
        setThumbnail(compactVideo.thumbnail.recommendedQuality(thumbnail->size())->url);
    else
        setThumbnail("https://img.youtube.com/vi/" + videoId + "/mqdefault.jpg");

    QString title = QString(compactVideo.title.text).replace("\r\n", " ");
    watchPreloadData->title = title;
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void VideoRenderer::setData(const InnertubeObjects::DisplayAd& displayAd,
                            bool useThumbnailFromData)
{
    videoEndpoint = displayAd.clickCommand;

    metadataLabel->setText(displayAd.bodyText.text);
    metadataLabel->setToolTip(displayAd.bodyText.text);

    if (displayAd.clickCommand.isObject())
    {
        channelLabel->show();
        channelLabel->setInfo(displayAd.clickCommand, displayAd.secondaryText.text);
    }

    thumbnail->setLengthText("Ad");
    setThumbnail(displayAd.image.recommendedQuality(thumbnail->size())->url);

    QString title = QString(displayAd.titleText.text).replace("\r\n", " ");
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void VideoRenderer::setData(const InnertubeObjects::LockupViewModel& lockup,
                            bool useThumbnailFromData)
{
    progress = lockup.rendererContext["commandContext"]["onTap"]["innertubeCommand"]
                                     ["watchEndpoint"]["startTimeSeconds"].toInt();
    videoId = lockup.contentId;
    watchPreloadData = std::make_unique<PreloadData::WatchView>();

    if (lockup.metadata.metadata.metadataRows.size() > 1)
    {
        QStringList metadataList;
        metadataList.reserve(2);
        for (const InnertubeObjects::DynamicText& part : lockup.metadata.metadata.metadataRows[1])
            metadataList.append(part.content);
        metadataLabel->setText(metadataList.join(lockup.metadata.metadata.delimiter));
    }

    if (std::optional<InnertubeObjects::BasicChannel> owner = lockup.owner())
    {
        watchPreloadData->channelAvatar = owner->icon;
        watchPreloadData->channelId = owner->id;
        watchPreloadData->channelName = owner->name;

        channelLabel->show();
        channelLabel->setInfo(owner->id, owner->name);
    }

    thumbnail->setLengthText(lockup.lengthText());
    thumbnail->setProgress(progress, QTime(0, 0).secsTo(lockup.length()));

    if (useThumbnailFromData && !lockup.contentImage.image.isEmpty())
        setThumbnail(lockup.contentImage.image.recommendedQuality(thumbnail->size())->url);
    else
        setThumbnail("https://img.youtube.com/vi/" + videoId + "/mqdefault.jpg");

    QString title = QString(lockup.metadata.title).replace("\r\n", " ");
    watchPreloadData->title = title;
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void VideoRenderer::setData(const InnertubeObjects::Reel& reel,
                            bool isInGrid, bool useThumbnailFromData)
{
    videoId = reel.videoId;

    channelLabel->deleteLater(); // no owner info, we're just gonna yeet this out of existence
    metadataLabel->setText(reel.viewCountText.text);

    thumbnail->setLengthText("SHORTS");

    if (isInGrid)
        thumbnail->setFixedSize(210, 372);
    else
        thumbnail->setFixedSize(105, 186);

    if (useThumbnailFromData && !reel.thumbnail.isEmpty())
        setThumbnail(reel.thumbnail.recommendedQuality(thumbnail->size())->url);
    else
        setThumbnail("https://img.youtube.com/vi/" + videoId + "/mqdefault.jpg");

    QString title = QString(reel.headline).replace("\r\n", " ");
    titleLabel->setText(title);
    titleLabel->setToolTip(title);

    watchPreloadData.reset(new PreloadData::WatchView { .title = title });
}

void VideoRenderer::setData(const InnertubeObjects::ShortsLockupViewModel& shortsLockup,
                            bool isInGrid, bool useThumbnailFromData)
{
    videoId = shortsLockup.videoId;

    channelLabel->deleteLater(); // no owner info, we're just gonna yeet this out of existence
    metadataLabel->setText(shortsLockup.secondaryText);

    thumbnail->setLengthText("SHORTS");

    if (isInGrid)
        thumbnail->setFixedSize(210, 372);
    else
        thumbnail->setFixedSize(105, 186);

    if (useThumbnailFromData && !shortsLockup.thumbnail.isEmpty())
        setThumbnail(shortsLockup.thumbnail.recommendedQuality(thumbnail->size())->url);
    else
        setThumbnail("https://img.youtube.com/vi/" + videoId + "/mqdefault.jpg");

    QString title = QString(shortsLockup.primaryText).replace("\r\n", " ");
    titleLabel->setText(title);
    titleLabel->setToolTip(title);

    watchPreloadData.reset(new PreloadData::WatchView { .title = title });
}

void VideoRenderer::setData(const InnertubeObjects::Video& video,
                            bool useThumbnailFromData)
{
    progress = video.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt();
    videoId = video.videoId;

    QStringList metadataList;
    metadataList.reserve(2);
    metadataList.append(qtTubeApp->settings().condensedCounts ? video.shortViewCountText.text : video.viewCountText.text);
    metadataList.append(video.publishedTimeDisplay());
    metadataList.removeAll({});
    metadataLabel->setText(metadataList.join(" • "));

    QString ownerId = video.ownerId();
    if (!ownerId.isEmpty())
    {
        channelLabel->show();
        channelLabel->setInfo(ownerId, video.ownerText.text, video.ownerBadges);
    }

    thumbnail->setLengthText(video.lengthDisplay().text);
    thumbnail->setProgress(progress, QTime(0, 0).secsTo(video.length()));

    if (useThumbnailFromData && !video.thumbnail.isEmpty())
        setThumbnail(video.thumbnail.recommendedQuality(thumbnail->size())->url);
    else
        setThumbnail("https://img.youtube.com/vi/" + videoId + "/mqdefault.jpg");

    QString title = QString(video.title.text).replace("\r\n", " ");
    titleLabel->setText(title);
    titleLabel->setToolTip(title);

    watchPreloadData.reset(new PreloadData::WatchView {
        .channelAvatar = video.channelThumbnailSupportedRenderers.thumbnail,
        .channelBadges = video.ownerBadges,
        .channelId = ownerId,
        .channelName = video.ownerText.text,
        .title = title
    });
}

void VideoRenderer::setData(const InnertubeObjects::VideoDisplayButtonGroup& video,
                            bool useThumbnailFromData)
{
    videoId = video.videoId;
    watchPreloadData = std::make_unique<PreloadData::WatchView>();
    metadataLabel->setText(video.badge.label);

    if (video.channelEndpoint.isObject())
    {
        watchPreloadData->channelAvatar = video.channelThumbnail;
        watchPreloadData->channelName = video.shortBylineText.text;

        channelLabel->show();
        channelLabel->setInfo(video.channelEndpoint, video.shortBylineText.text);
    }

    auto overlayIt = std::ranges::find_if(video.thumbnailOverlays, [](const InnertubeObjects::ThumbnailOverlay& overlay) {
        return std::holds_alternative<InnertubeObjects::ThumbnailOverlayTimeStatus>(overlay);
    });

    if (overlayIt != video.thumbnailOverlays.end())
    {
        const auto& timeOverlay = std::get<InnertubeObjects::ThumbnailOverlayTimeStatus>(*overlayIt);
        if (timeOverlay.iconType == "EXTERNAL_LINK")
            thumbnail->setLengthText("Ad");
        else
            thumbnail->setLengthText(timeOverlay.text.text);
    }
    else
    {
        thumbnail->setLengthText(video.lengthText.text);
    }

    if (useThumbnailFromData && !video.thumbnail.isEmpty())
        setThumbnail(video.thumbnail.recommendedQuality(thumbnail->size())->url);
    else
        setThumbnail("https://img.youtube.com/vi/" + videoId + "/mqdefault.jpg");

    QString title = QString(video.title.text).replace("\r\n", " ");
    watchPreloadData->title = title;
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void VideoRenderer::setData(const QtTube::PluginVideo& video)
{
    progress = video.progressSecs;
    videoId = video.videoId;

    metadataLabel->setText(video.metadataText);
    if (!video.uploaderId.isEmpty())
    {
        channelLabel->show();
        channelLabel->setInfo(video.uploaderId, video.uploaderText, video.uploaderBadges);
    }

    thumbnail->setLengthText(video.lengthText);
    thumbnail->setProgress(progress, QTime(0, 0).secsTo(video.length()));
    thumbnail->setSourceIconUrl(video.sourceMetadata->image);

    if (strcmp(video.sourceMetadata->name, "YouTube") == 0)
        setThumbnail(video.thumbnailUrl);
    else
        thumbnail->setImage(video.thumbnailUrl);

    titleLabel->setText(video.title);
    titleLabel->setToolTip(video.title);

    watchPreloadData.reset(new PreloadData::WatchView {
        //.channelAvatar = video.uploaderAvatarUrl,
        //.channelBadges = video.uploaderBadges,
        .channelId = video.uploaderId,
        .channelName = video.uploaderText,
        .title = video.title
    });
}

void VideoRenderer::setDeArrowData(const QString& thumbFallbackUrl, const HttpReply& reply)
{
    if (!reply.isSuccessful())
    {
        thumbnail->setImage(thumbFallbackUrl);
        return;
    }

    QJsonValue obj = QJsonDocument::fromJson(reply.body()).object();
    const QJsonArray titles = obj["titles"].toArray();
    const QJsonArray thumbs = obj["thumbnails"].toArray();

    auto validReplacement = [](const QJsonArray& a) {
        return !a.isEmpty() && (a[0]["locked"].toBool() || a[0]["votes"].toInt() >= 0);
    };

    if (qtTubeApp->settings().deArrowTitles && validReplacement(titles))
    {
        // for some reason, a lot of dearrow titles have unnecessary >s.
        // i haven't looked into it much but i'm just going to manually
        // remove them for now.
        QString title = titles[0]["title"].toString().replace(">", "");
        titleLabel->setText(title);
        titleLabel->setToolTip(title);
    }

    if (qtTubeApp->settings().deArrowThumbs && validReplacement(thumbs))
        thumbnail->setImage(QStringLiteral("https://dearrow-thumb.ajay.app/api/v1/getThumbnail?videoID=%1&timestamp=%2").arg(videoId).arg(thumbs[0]["timestamp"].toDouble()));
    else
        thumbnail->setImage(thumbFallbackUrl);
}

void VideoRenderer::setThumbnail(const QString& url)
{
    if (qtTubeApp->settings().deArrow)
    {
        HttpReply* arrowReply = Http::instance().get("https://sponsor.ajay.app/api/branding?videoID=" + videoId);
        connect(arrowReply, &HttpReply::finished, this, std::bind_front(&VideoRenderer::setDeArrowData, this, url));
    }
    else
    {
        thumbnail->setImage(url);
    }
}

void VideoRenderer::showTitleContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyDirectAction = new QAction("Copy direct video URL", this);
    connect(copyDirectAction, &QAction::triggered, this, &VideoRenderer::copyDirectUrl);

    QAction* copyUrlAction = new QAction("Copy video page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &VideoRenderer::copyVideoUrl);

    menu->addAction(copyUrlAction);
    menu->addAction(copyDirectAction);
    menu->popup(titleLabel->mapToGlobal(pos));
}
