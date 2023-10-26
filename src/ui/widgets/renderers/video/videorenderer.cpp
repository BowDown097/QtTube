#include "videorenderer.h"
#include "http.h"
#include "innertube.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include "ui/views/viewcontroller.h"
#include <QMenu>
#include <QMessageBox>

VideoRenderer::VideoRenderer(QWidget* parent)
    : QWidget(parent),
      channelLabel(new ChannelLabel(this)),
      metadataLabel(new TubeLabel(this)),
      thumbLabel(new TubeLabel(this)),
      titleLabel(new ElidedTubeLabel(this))
{
    titleLabel->setClickable(true, true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2, QFont::Bold));

    thumbLabel->setClickable(true, false);
    thumbLabel->setMinimumSize(1, 1);
    thumbLabel->setScaledContents(true);

    connect(channelLabel->text, &TubeLabel::clicked, this, &VideoRenderer::navigateChannel);
    connect(channelLabel->text, &TubeLabel::customContextMenuRequested, this, &VideoRenderer::showChannelContextMenu);
    connect(thumbLabel, &TubeLabel::clicked, this, &VideoRenderer::navigateVideo);
    connect(titleLabel, &ElidedTubeLabel::clicked, this, &VideoRenderer::navigateVideo);
    connect(titleLabel, &TubeLabel::customContextMenuRequested, this, &VideoRenderer::showTitleContextMenu);
}

void VideoRenderer::copyChannelUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/channel/" + channelId);
}

void VideoRenderer::copyDirectUrl()
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId);
    connect(reply, &InnertubeReply::exception, this, [this] {
        QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely unavailable.");
    });
    connect(reply, qOverload<const InnertubeEndpoints::Player&>(&InnertubeReply::finished), this, [this](const InnertubeEndpoints::Player& endpoint) {
        if (endpoint.response.videoDetails.isLive || endpoint.response.videoDetails.isLiveContent)
        {
            UIUtils::copyToClipboard(endpoint.response.streamingData.hlsManifestUrl);
        }
        else
        {
            QList<InnertubeObjects::StreamingFormat>::const_iterator best = std::ranges::max_element(
                endpoint.response.streamingData.formats,
                [](const auto& a, const auto& b) { return a.bitrate < b.bitrate; }
            );

            if (best == endpoint.response.streamingData.formats.end())
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
    ViewController::loadVideo(videoId, progress);
}

void VideoRenderer::setData(const InnertubeObjects::Reel& reel)
{
    channelId = reel.owner.id;
    videoId = reel.videoId;

    QString title = QString(reel.headline).replace("\r\n", " ");

    channelLabel->setInfo(reel.owner.name, reel.owner.badges);
    metadataLabel->setText("SHORTS • " + reel.viewCountText.text);
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void VideoRenderer::setData(const InnertubeObjects::Video& video)
{
    channelId = video.owner.id;
    progress = video.navigationEndpoint["watchEndpoint"]["startTimeSeconds"].toInt();
    videoId = video.videoId;

    QString progStr = QStringLiteral("%1 watched")
        .arg(QDateTime::fromSecsSinceEpoch(progress, Qt::UTC).toString(progress >= 3600 ? "h:mm:ss" : "m:ss"));
    QString title = QString(video.title.text).replace("\r\n", " ");

    QStringList metadataList {
        video.lengthText.text,
        video.publishedTimeText.text,
        qtTubeApp->settings().condensedViews ? video.shortViewCountText.text : video.viewCountText.text
    };
    if (progress != 0) metadataList.append(progStr);
    metadataList.removeAll({});

    channelLabel->setInfo(video.owner.name, video.owner.badges);
    metadataLabel->setText(metadataList.join(" • "));
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void VideoRenderer::setDeArrowData(const HttpReply& reply, const QString& thumbFallbackUrl)
{
    if (!reply.isSuccessful())
    {
        HttpReply* reply = Http::instance().get(thumbFallbackUrl);
        connect(reply, &HttpReply::finished, this, &VideoRenderer::setThumbnailData);
        return;
    }

    QJsonValue obj = QJsonDocument::fromJson(reply.body()).object();
    const QJsonArray titles = obj["titles"].toArray();
    const QJsonArray thumbs = obj["thumbnails"].toArray();

    if (qtTubeApp->settings().deArrowTitles && !titles.isEmpty() && (titles[0]["locked"].toBool() || titles[0]["votes"].toInt() >= 0))
    {
        titleLabel->setText(titles[0]["title"].toString());
        titleLabel->setToolTip(titles[0]["title"].toString());
    }

    if (qtTubeApp->settings().deArrowThumbs && !thumbs.isEmpty() && (thumbs[0]["locked"].toBool() || thumbs[0]["votes"].toInt() >= 0))
    {
        HttpReply* reply = Http::instance().get(QStringLiteral("https://dearrow-thumb.ajay.app/api/v1/getThumbnail?videoID=%1&timestamp=%2")
                                                    .arg(videoId).arg(thumbs[0]["timestamp"].toDouble()));
        connect(reply, &HttpReply::finished, this, &VideoRenderer::setThumbnailData);
    }
    else
    {
        HttpReply* reply = Http::instance().get(thumbFallbackUrl);
        connect(reply, &HttpReply::finished, this, &VideoRenderer::setThumbnailData);
    }
}

void VideoRenderer::setThumbnail(const QString& url)
{
    if (qtTubeApp->settings().deArrow)
    {
        HttpReply* arrowReply = Http::instance().get("https://sponsor.ajay.app/api/branding?videoID=" + videoId);
        connect(arrowReply, &HttpReply::finished, this, std::bind(&VideoRenderer::setDeArrowData, this, std::placeholders::_1, url));
    }
    else
    {
        HttpReply* reply = Http::instance().get(url);
        connect(reply, &HttpReply::finished, this, &VideoRenderer::setThumbnailData);
    }
}

void VideoRenderer::setThumbnailData(const HttpReply& reply)
{
    if (reply.statusCode() != 200)
        return;

    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    QSize size = thumbnailSize.height() == 0 ? QSize(thumbnailSize.width(), thumbLabel->height()) : thumbnailSize;
    thumbLabel->setPixmap(pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    if (targetElisionWidth > 0)
        UIUtils::elide(titleLabel, targetElisionWidth);
}

void VideoRenderer::setThumbnailSize(const QSize& size)
{
    thumbnailSize = size;
    if (size.height() > 0)
    {
        metadataLabel->setFixedWidth(size.width());
        thumbLabel->setFixedSize(size);
        titleLabel->setFixedWidth(size.width());
    }
}

void VideoRenderer::showChannelContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &VideoRenderer::copyChannelUrl);

    menu->addAction(copyUrlAction);
    menu->popup(channelLabel->mapToGlobal(pos));
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
