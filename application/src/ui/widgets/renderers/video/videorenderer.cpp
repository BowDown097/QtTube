#include "videorenderer.h"
#include "plugins/pluginmanager.h"
#include "ui/views/preloaddata.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "utils/uiutils.h"
#include "videothumbnailwidget.h"
#include <QMenu>

VideoRenderer::VideoRenderer(PluginData* plugin, QWidget* parent)
    : QWidget(parent),
      channelLabel(new ChannelLabel(plugin, this)),
      metadataLabel(new TubeLabel(this)),
      thumbnail(new VideoThumbnailWidget(this)),
      titleLabel(new TubeLabel(this)),
      plugin(plugin)
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
    ViewController::loadVideo(videoId, plugin, progress, watchPreloadData.get());
}

void VideoRenderer::setData(const QtTubePlugin::Video& video)
{
    progress = video.progressSecs;
    videoId = video.videoId;

    metadataLabel->setText(video.metadataText);
    if (!video.uploaderId.isEmpty() || !video.uploaderText.isEmpty())
    {
        channelLabel->show();
        channelLabel->setInfo(video.uploaderId, video.uploaderText, video.uploaderBadges);
    }

    thumbnail->setLengthText(video.lengthText);
    thumbnail->setProgress(progress, QTime(0, 0).secsTo(video.length()));
    thumbnail->setSourceIconUrl(plugin->metadata.image);

    if (video.isVerticalVideo)
        thumbnail->setFixedSize(105, 186);

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

void VideoRenderer::showTitleContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy video page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &VideoRenderer::copyVideoUrl);

    menu->addAction(copyUrlAction);
    menu->popup(titleLabel->mapToGlobal(pos));
}
