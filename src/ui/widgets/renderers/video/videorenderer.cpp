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

    connect(channelLabel->text, &TubeLabel::clicked, this, &VideoRenderer::navigateChannel);
    connect(thumbnail, &VideoThumbnailWidget::clicked, this, &VideoRenderer::navigateVideo);
    connect(titleLabel, &TubeLabel::clicked, this, &VideoRenderer::navigateVideo);
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
            auto best = std::ranges::max_element(streamingData.formats, [](const auto& a, const auto& b) {
                return a.bitrate < b.bitrate;
            });

            if (best == streamingData.formats.end())
            {
                QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely unavailable.");
                return;
            }

            UIUtils::copyToClipboard(best->url);
        }
    });
}

void VideoRenderer::copyVideoUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/watch?v=" + videoId);
}

void VideoRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void VideoRenderer::navigateVideo()
{
    ViewController::loadVideo(videoId, progress, watchPreloadData.get());
}

void VideoRenderer::setData(const InnertubeObjects::CompactVideo& compactVideo,
                            bool useThumbnailFromData)
{
    progress = compactVideo.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt();
    videoId = compactVideo.videoId;
    watchPreloadData = std::make_unique<PreloadData::WatchView>();

    QStringList metadataList;
    metadataList.reserve(2);
    metadataList.append(compactVideo.publishedTimeText.text);
    metadataList.append(qtTubeApp->settings().condensedCounts
        ? compactVideo.shortViewCountText.text : compactVideo.viewCountText.text);
    metadataList.removeAll({});
    metadataLabel->setText(metadataList.join(" • "));

    InnertubeObjects::VideoOwner owner = compactVideo.owner();

    if (InnertubeObjects::VideoOwner owner = compactVideo.owner(); !owner.id.isEmpty())
    {
        channelId = owner.id;
        watchPreloadData->channelAvatar = owner.icon;
        watchPreloadData->channelBadges = compactVideo.ownerBadges;
        watchPreloadData->channelId = owner.id;
        watchPreloadData->channelName = owner.name;

        channelLabel->show();
        channelLabel->setInfo(channelId, owner.name, compactVideo.ownerBadges);
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

    if (std::optional<InnertubeObjects::VideoOwner> owner = lockup.owner())
    {
        channelId = owner->id;
        watchPreloadData->channelAvatar = owner->icon;
        watchPreloadData->channelId = owner->id;
        watchPreloadData->channelName = owner->name;

        channelLabel->show();
        channelLabel->setInfo(channelId, owner->name, {});
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
    channelId = video.ownerId();
    progress = video.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt();
    videoId = video.videoId;

    QStringList metadataList;
    metadataList.reserve(2);
    metadataList.append(video.publishedTimeDisplay());
    metadataList.append(qtTubeApp->settings().condensedCounts ? video.shortViewCountText.text : video.viewCountText.text);
    metadataList.removeAll({});
    metadataLabel->setText(metadataList.join(" • "));

    if (!channelId.isEmpty())
    {
        channelLabel->show();
        channelLabel->setInfo(channelId, video.ownerText.text, video.ownerBadges);
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
        .channelId = channelId,
        .channelName = video.ownerText.text,
        .title = title
    });
}

void VideoRenderer::setDeArrowData(const QString& thumbFallbackUrl, const HttpReply& reply)
{
    if (!reply.isSuccessful())
    {
        thumbnail->setUrl(thumbFallbackUrl);
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
        thumbnail->setUrl(QStringLiteral("https://dearrow-thumb.ajay.app/api/v1/getThumbnail?videoID=%1&timestamp=%2").arg(videoId).arg(thumbs[0]["timestamp"].toDouble()));
    else
        thumbnail->setUrl(thumbFallbackUrl);
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
        thumbnail->setUrl(url);
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
