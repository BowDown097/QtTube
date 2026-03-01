#include "videorenderer.h"
#include "plugins/pluginentry.h"
#include "ui/views/preloaddata.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/badgelabel.h"
#include "ui/widgets/labels/channellabel.h"
#include "utils/uiutils.h"
#include "videothumbnailwidget.h"
#include <QBoxLayout>
#include <QMenu>

VideoRenderer::VideoRenderer(PluginEntry* plugin, QWidget* parent)
    : QWidget(parent),
      badgesLayout(new QHBoxLayout),
      channelLabel(new ChannelLabel(plugin, this)),
      metadataLabel(new TubeLabel(this)),
      thumbnail(new VideoThumbnailWidget(this)),
      titleLabel(new TubeLabel(this)),
      m_plugin(plugin)
{
    channelLabel->addStretch();
    channelLabel->hide();

    titleLabel->setClickable(true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setElideMode(Qt::ElideRight);
    titleLabel->setFont(QFont(font().toString(), font().pointSize() + 2, QFont::Bold));
    titleLabel->setUnderlineOnHover(true);

    badgesLayout->setContentsMargins(0, 0, 0, 0);

    connect(thumbnail, &VideoThumbnailWidget::clicked, this, &VideoRenderer::navigate);
    connect(titleLabel, &TubeLabel::clicked, this, &VideoRenderer::navigate);
    connect(titleLabel, &TubeLabel::customContextMenuRequested, this, &VideoRenderer::showTitleContextMenu);
}

void VideoRenderer::copyVideoUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/watch?v=" + m_videoId);
}

void VideoRenderer::navigate()
{
    ViewController::loadVideo(m_videoId, m_plugin, m_progress, m_preloadData.get());
}

void VideoRenderer::setData(const QtTubePlugin::Video& video)
{
    m_progress = video.progressSecs;
    m_videoId = video.videoId;

    metadataLabel->setText(video.metadataText);
    if (!video.uploaderId.isEmpty() || !video.uploaderText.isEmpty())
    {
        channelLabel->show();
        channelLabel->setInfo(video.uploaderId, video.uploaderText, video.uploaderBadges);
    }

    thumbnail->setLengthText(video.lengthText);
    thumbnail->setProgress(m_progress, QTime(0, 0).secsTo(video.length()));
    thumbnail->setSourceIconUrl(m_plugin->metadata.image);

    if (video.isVerticalVideo)
        thumbnail->setFixedSize(105, 186);

    thumbnail->setImage(video.thumbnailUrl, TubeLabel::LazyLoaded);

    titleLabel->setText(video.title);
    titleLabel->setToolTip(video.title);

    for (const QtTubePlugin::Badge& badge : video.badges)
    {
        BadgeLabel* badgeLabel = new BadgeLabel(this);
        badgeLabel->setData(badge);
        badgesLayout->addWidget(badgeLabel);
    }

    badgesLayout->addStretch();

    m_preloadData.reset(new PreloadData::WatchView {
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
