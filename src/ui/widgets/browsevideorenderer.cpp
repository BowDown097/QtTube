#include "browsevideorenderer.h"
#include "innertube.h"
#include "ui/uiutilities.h"
#include "ui/views/channelview.h"
#include "ui/views/watchview.h"
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

BrowseVideoRenderer::BrowseVideoRenderer(QWidget* parent) : QWidget(parent)
{
    channelLabel = new TubeLabel;
    hbox = new QHBoxLayout;
    metadataLabel = new TubeLabel;
    textVbox = new QVBoxLayout;
    thumbLabel = new TubeLabel;
    titleLabel = new TubeLabel;

    textVbox->addWidget(titleLabel);
    textVbox->addWidget(channelLabel);
    textVbox->addWidget(metadataLabel);

    hbox->addWidget(thumbLabel);
    hbox->addLayout(textVbox, 1);
    setLayout(hbox);

    channelLabel->setClickable(true, true);
    channelLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    thumbLabel->setClickable(true, false);
    thumbLabel->setMinimumSize(1, 1);
    thumbLabel->setScaledContents(true);

    titleLabel->setClickable(true, true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));

    connect(channelLabel, &TubeLabel::clicked, this, &BrowseVideoRenderer::navigateChannel);
    connect(channelLabel, &TubeLabel::customContextMenuRequested, this, &BrowseVideoRenderer::showChannelContextMenu);
    connect(thumbLabel, &TubeLabel::clicked, this, &BrowseVideoRenderer::navigateVideo);
    connect(titleLabel, &TubeLabel::clicked, this, &BrowseVideoRenderer::navigateVideo);
    connect(titleLabel, &TubeLabel::customContextMenuRequested, this, &BrowseVideoRenderer::showTitleContextMenu);
}

void BrowseVideoRenderer::copyChannelUrl()
{
    UIUtilities::copyToClipboard("https://www.youtube.com/channel/" + channelId);
}

void BrowseVideoRenderer::copyDirectUrl()
{
    auto response = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId).response;
    if (response.videoDetails.isLive || response.videoDetails.isLiveContent)
    {
        UIUtilities::copyToClipboard(response.streamingData.hlsManifestUrl);
    }
    else
    {
        QList<InnertubeObjects::StreamingFormat>::const_iterator best = std::max_element(
            response.streamingData.formats.cbegin(), response.streamingData.formats.cend(),
            [](const auto& a, const auto& b) { return a.bitrate < b.bitrate; }
        );

        if (best == response.streamingData.formats.cend())
        {
            QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely uanvailable.");
            return;
        }

        UIUtilities::copyToClipboard((*best).url);
    }
}

void BrowseVideoRenderer::copyVideoUrl()
{
    UIUtilities::copyToClipboard("https://www.youtube.com/watch?v=" + videoId);
}

void BrowseVideoRenderer::navigateChannel()
{
    try
    {
        ChannelView::instance()->loadChannel(channelId);
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(this, "Failed to load channel", ie.message());
    }
}

void BrowseVideoRenderer::navigateVideo()
{
    try
    {
        WatchView::instance()->loadVideo(videoId, progress);
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(this, "Failed to load video", ie.message());
    }
}

void BrowseVideoRenderer::setChannelData(const InnertubeObjects::VideoOwner& owner)
{
    channelId = owner.id;
    channelLabel->setText(owner.name);
}

void BrowseVideoRenderer::setThumbnail(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap.scaled(240, thumbLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void BrowseVideoRenderer::setVideoData(const QString& length, const QString& publishedTime, int progress, const QString& title,
                                       const QString& videoId, const QString& viewCount)
{
    this->progress = progress;
    this->videoId = videoId;

    QString progStr = QStringLiteral("%1 watched")
            .arg(QDateTime::fromSecsSinceEpoch(progress, Qt::UTC)
            .toString(progress >= 3600 ? "h:mm:ss" : "m:ss"));

    QStringList list({length, publishedTime, viewCount});
    if (progress != 0) list.append(progStr);
    list.removeAll({});

    metadataLabel->setText(list.join(" • "));
    titleLabel->setText(title.length() <= 60 ? title : title.left(60) + "…");
    titleLabel->setToolTip(title);
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
