#include "videorenderer.h"
#include "httprequest.h"
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
        .channelBadges = video.ownerBadges,
        .channelId = ownerId,
        .channelName = video.ownerText.text,
        .title = title
    });

    const InnertubeObjects::ResponsiveImage& thumbnail = video.channelThumbnailSupportedRenderers.thumbnail;
    if (const InnertubeObjects::GenericThumbnail* recThumb = thumbnail.recommendedQuality(QSize(48, 48)))
        watchPreloadData->channelAvatarUrl = recThumb->url;
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

    if (video.isVerticalVideo)
        thumbnail->setFixedSize(105, 186);

    if (strcmp(video.sourceMetadata->name, "YouTube") == 0)
        setThumbnail(video.thumbnailUrl);
    else
        thumbnail->setImage(video.thumbnailUrl);

    titleLabel->setText(video.title);
    titleLabel->setToolTip(video.title);

    watchPreloadData.reset(new PreloadData::WatchView {
        .channelAvatarUrl = video.uploaderAvatarUrl,
        .channelBadges = video.uploaderBadges,
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

    QJsonValue obj = QJsonDocument::fromJson(reply.readAll()).object();
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
        HttpReply* reply = HttpRequest().get("https://sponsor.ajay.app/api/branding?videoID=" + videoId);
        connect(reply, &HttpReply::finished, this, std::bind_front(&VideoRenderer::setDeArrowData, this, url));
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
