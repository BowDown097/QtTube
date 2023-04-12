#include "browsevideorenderer.h"
#include "innertube.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include "ui/views/channelview.h"
#include "ui/views/viewcontroller.h"
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

BrowseVideoRenderer::BrowseVideoRenderer(QWidget* parent) : QWidget(parent)
{
    hbox = new QHBoxLayout(this);
    setLayout(hbox);

    textVbox = new QVBoxLayout(this);

    titleLabel = new TubeLabel(this);
    titleLabel->setClickable(true, true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));
    textVbox->addWidget(titleLabel);

    channelLabel = new ChannelLabel(this);
    textVbox->addWidget(channelLabel);

    metadataLabel = new TubeLabel(this);
    textVbox->addWidget(metadataLabel);

    thumbLabel = new TubeLabel(this);
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
    UIUtilities::copyToClipboard("https://www.youtube.com/channel/" + channelId);
}

void BrowseVideoRenderer::copyDirectUrl()
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId);
    connect(reply, &InnertubeReply::exception, this, [this]
    {
        QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely unavailable.");
    });
    connect(reply, qOverload<InnertubeEndpoints::Player>(&InnertubeReply::finished), this, [this](const auto& endpoint)
    {
        if (endpoint.response.videoDetails.isLive || endpoint.response.videoDetails.isLiveContent)
        {
            UIUtilities::copyToClipboard(endpoint.response.streamingData.hlsManifestUrl);
        }
        else
        {
            QList<InnertubeObjects::StreamingFormat>::const_iterator best = std::ranges::max_element(
                endpoint.response.streamingData.formats,
                [](const auto& a, const auto& b) { return a.bitrate < b.bitrate; }
            );

            if (best == endpoint.response.streamingData.formats.cend())
            {
                QMessageBox::critical(this, "Failed to copy to clipboard", "Failed to copy the direct video URL to the clipboard. The video is likely unavailable.");
                return;
            }

            UIUtilities::copyToClipboard((*best).url);
        }
    });
}

void BrowseVideoRenderer::copyVideoUrl()
{
    UIUtilities::copyToClipboard("https://www.youtube.com/watch?v=" + videoId);
}

void BrowseVideoRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void BrowseVideoRenderer::navigateVideo()
{
    if (ChannelView* channelView = qobject_cast<ChannelView*>(MainWindow::centralWidget()->currentWidget()))
        channelView->deleteLater();
    ViewController::loadVideo(videoId, progress);
}

void BrowseVideoRenderer::setChannelData(const InnertubeObjects::VideoOwner& owner)
{
    channelId = owner.id;
    channelLabel->setInfo(owner.name, owner.badges);
}

void BrowseVideoRenderer::setThumbnail(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap.scaled(240, thumbLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    UIUtilities::elide(titleLabel, targetElisionWidth);
}

void BrowseVideoRenderer::setVideoData(const QString& length, const QString& publishedTime, int progress, QString title,
                                       const QString& videoId, const QString& viewCount)
{
    this->progress = progress;
    this->videoId = videoId;
    title.replace("\r\n", " ");

    QString progStr = QStringLiteral("%1 watched")
            .arg(QDateTime::fromSecsSinceEpoch(progress, Qt::UTC)
            .toString(progress >= 3600 ? "h:mm:ss" : "m:ss"));

    QStringList list({length, publishedTime, viewCount});
    if (progress != 0) list.append(progStr);
    list.removeAll({});

    metadataLabel->setText(list.join(" • "));
    titleLabel->setText(title);
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
