#include "videorenderer.h"
#include "httprequest.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include "ui/views/preloaddata.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "videothumbnailwidget.h"
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

void VideoRenderer::copyVideoUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/watch?v=" + videoId);
}

void VideoRenderer::navigate()
{
    ViewController::loadVideo(videoId, progress, watchPreloadData.get());
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

    QAction* copyUrlAction = new QAction("Copy video page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &VideoRenderer::copyVideoUrl);

    menu->addAction(copyUrlAction);
    menu->popup(titleLabel->mapToGlobal(pos));
}
