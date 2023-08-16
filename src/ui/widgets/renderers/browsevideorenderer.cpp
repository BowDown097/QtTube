#include "browsevideorenderer.h"
#include "http.h"
#include "innertube.h"
#include "stores/settingsstore.h"
#include "utils/uiutils.h"
#include "ui/views/viewcontroller.h"
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

BrowseVideoRenderer::BrowseVideoRenderer(QWidget* parent)
    : QWidget(parent),
      channelLabel(new ChannelLabel(this)),
      hbox(new QHBoxLayout(this)),
      metadataLabel(new TubeLabel(this)),
      textVbox(new QVBoxLayout),
      thumbLabel(new TubeLabel(this)),
      titleLabel(new TubeLabel(this))
{
    titleLabel->setClickable(true, true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2, QFont::Bold));
    textVbox->addWidget(titleLabel);

    textVbox->addWidget(channelLabel);
    textVbox->addWidget(metadataLabel);

    thumbLabel->setClickable(true, false);
    thumbLabel->setMinimumSize(1, 1);
    thumbLabel->setScaledContents(true);
    hbox->addWidget(thumbLabel);

    hbox->addLayout(textVbox, 1);

    connect(channelLabel->text, &TubeLabel::clicked, this, &BrowseVideoRenderer::navigateChannel);
    connect(channelLabel->text, &TubeLabel::customContextMenuRequested, this, &BrowseVideoRenderer::showChannelContextMenu);
    connect(thumbLabel, &TubeLabel::clicked, this, &BrowseVideoRenderer::navigateVideo);
    connect(titleLabel, &TubeLabel::clicked, this, &BrowseVideoRenderer::navigateVideo);
    connect(titleLabel, &TubeLabel::customContextMenuRequested, this, &BrowseVideoRenderer::showTitleContextMenu);
}

void BrowseVideoRenderer::copyChannelUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/channel/" + channelId);
}

void BrowseVideoRenderer::copyDirectUrl()
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId);
    connect(reply, &InnertubeReply::exception, this, [this]
    {
        QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely unavailable.");
    });
    connect(reply, qOverload<const InnertubeEndpoints::Player&>(&InnertubeReply::finished), this, [this](const InnertubeEndpoints::Player& endpoint)
    {
        if (endpoint.response.videoDetails.isLive || endpoint.response.videoDetails.isLiveContent)
        {
            UIUtils::copyToClipboard(endpoint.response.streamingData.hlsManifestUrl);
        }
        else
        {
            auto best = std::ranges::max_element(
                endpoint.response.streamingData.formats,
                [](const InnertubeObjects::StreamingFormat& a, const InnertubeObjects::StreamingFormat& b) { return a.bitrate < b.bitrate; }
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

void BrowseVideoRenderer::copyVideoUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/watch?v=" + videoId);
}

void BrowseVideoRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void BrowseVideoRenderer::navigateVideo()
{
    ViewController::loadVideo(videoId, progress);
}

void BrowseVideoRenderer::setData(const InnertubeObjects::Reel& reel)
{
    channelId = reel.owner.id;
    videoId = reel.videoId;

    QString title = QString(reel.headline).replace("\r\n", " ");

    channelLabel->setInfo(reel.owner.name, reel.owner.badges);
    metadataLabel->setText("SHORTS • " + reel.viewCountText.text);
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void BrowseVideoRenderer::setData(const InnertubeObjects::Video& video)
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
        SettingsStore::instance()->condensedViews ? video.shortViewCountText.text : video.viewCountText.text
    };
    if (progress != 0) metadataList.append(progStr);
    metadataList.removeAll({});

    channelLabel->setInfo(video.owner.name, video.owner.badges);
    metadataLabel->setText(metadataList.join(" • "));
    titleLabel->setText(title);
    titleLabel->setToolTip(title);
}

void BrowseVideoRenderer::setDeArrowData(const HttpReply& reply, const QString& thumbFallbackUrl)
{
    if (!reply.isSuccessful())
    {
        HttpReply* reply = Http::instance().get(thumbFallbackUrl);
        connect(reply, &HttpReply::finished, this, &BrowseVideoRenderer::setThumbnailData);
        return;
    }

    QJsonValue obj = QJsonDocument::fromJson(reply.body()).object();
    const QJsonArray titles = obj["titles"].toArray();
    const QJsonArray thumbs = obj["thumbnails"].toArray();

    if (SettingsStore::instance()->deArrowTitles && !titles.isEmpty() && (titles[0]["locked"].toBool() || titles[0]["votes"].toInt() >= 0))
    {
        titleLabel->setText(titles[0]["title"].toString());
        titleLabel->setToolTip(titles[0]["title"].toString());
    }

    if (SettingsStore::instance()->deArrowThumbs && !thumbs.isEmpty() && (thumbs[0]["locked"].toBool() || thumbs[0]["votes"].toInt() >= 0))
    {
        HttpReply* reply = Http::instance().get(QStringLiteral("https://dearrow-thumb.ajay.app/api/v1/getThumbnail?videoID=%1&timestamp=%2")
                                                    .arg(videoId).arg(thumbs[0]["timestamp"].toDouble()));
        connect(reply, &HttpReply::finished, this, &BrowseVideoRenderer::setThumbnailData);
    }
    else
    {
        HttpReply* reply = Http::instance().get(thumbFallbackUrl);
        connect(reply, &HttpReply::finished, this, &BrowseVideoRenderer::setThumbnailData);
    }
}

void BrowseVideoRenderer::setThumbnail(const QString& url)
{
    if (SettingsStore::instance()->deArrow)
    {
        HttpReply* arrowReply = Http::instance().get("https://sponsor.ajay.app/api/branding?videoID=" + videoId);
        connect(arrowReply, &HttpReply::finished, this, std::bind(&BrowseVideoRenderer::setDeArrowData, this, std::placeholders::_1, url));
    }
    else
    {
        HttpReply* reply = Http::instance().get(url);
        connect(reply, &HttpReply::finished, this, &BrowseVideoRenderer::setThumbnailData);
    }
}

void BrowseVideoRenderer::setThumbnailData(const HttpReply& reply)
{
    if (reply.statusCode() != 200)
        return;

    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap.scaled(240, thumbLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    UIUtils::elide(titleLabel, targetElisionWidth);
}

void BrowseVideoRenderer::showChannelContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &BrowseVideoRenderer::copyChannelUrl);

    menu->addAction(copyUrlAction);
    menu->popup(channelLabel->mapToGlobal(pos));
}

void BrowseVideoRenderer::showTitleContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyDirectAction = new QAction("Copy direct video URL", this);
    connect(copyDirectAction, &QAction::triggered, this, &BrowseVideoRenderer::copyDirectUrl);
    QAction* copyUrlAction = new QAction("Copy video page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &BrowseVideoRenderer::copyVideoUrl);

    menu->addAction(copyUrlAction);
    menu->addAction(copyDirectAction);
    menu->popup(titleLabel->mapToGlobal(pos));
}
