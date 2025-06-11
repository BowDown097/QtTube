#include "watchnextfeed.h"
#include "continuablelistwidget.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "ui/widgets/renderers/video/videothumbnailwidget.h"
#include "utils/uiutils.h"

WatchNextFeed::WatchNextFeed(QWidget* parent)
    : QTabWidget(parent),
      comments(new ContinuableListWidget(this)),
      recommended(new ContinuableListWidget(this))
{
    addTab(recommended, "Recommended");
    addTab(comments, "Comments");
    setTabVisible(1, false);
}

void WatchNextFeed::continueComments()
{
}

void WatchNextFeed::continueRecommended()
{
}

void WatchNextFeed::reset()
{
    setCurrentIndex(0);
    setTabVisible(1, false);
    comments->clear();
    recommended->clear();
}

void WatchNextFeed::setData(
    const QList<QtTube::PluginVideo>& recommendedVideos,
    const QtTube::VideoData::Continuations& continuations)
{
    if (continuations.comments.has_value())
    {
        setTabVisible(1, true);
        commentsContinuation = continuations.comments;
        connect(comments, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueComments);
    }

    if (continuations.recommended.has_value())
    {
        recommendedContinuation = continuations.recommended;
        connect(recommended, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueRecommended);
    }

    for (const QtTube::PluginVideo& video : recommendedVideos)
    {
        BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
        renderer->thumbnail->setFixedSize(167, 94);
        renderer->titleLabel->setMaximumLines(2);
        renderer->titleLabel->setWordWrap(true);
        renderer->setData(video);

        UIUtils::addWidgetToList(recommended, renderer);
        QCoreApplication::processEvents();
    }
}
