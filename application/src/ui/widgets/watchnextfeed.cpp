#include "watchnextfeed.h"
#include "continuablelistwidget.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "ui/widgets/renderers/video/videothumbnailwidget.h"
#include "utils/uiutils.h"
#include <QMessageBox>

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
    if (PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::RecommendedContinuationReply* reply = activePlugin->interface->continueRecommended(
                videoId, recommended->continuationData))
        {
            recommended->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::RecommendedContinuationReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
                recommended->setPopulatingFlag(false);
                QMessageBox::critical(nullptr, "Failed to get continuation data", ex.message());
            });
            connect(reply, &QtTubePlugin::RecommendedContinuationReply::finished,
                    this, &WatchNextFeed::continueRecommendedFinished);
        }
    }
}

void WatchNextFeed::continueRecommendedFinished(const QtTubePlugin::RecommendedContinuationData& data)
{
    recommended->continuationData = data.nextContinuation;
    populateRecommended(data.videos);
    recommended->setPopulatingFlag(false);
}

void WatchNextFeed::populateRecommended(const QList<QtTubePlugin::Video>& videos)
{
    for (const QtTubePlugin::Video& video : videos)
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

void WatchNextFeed::reset()
{
    setCurrentIndex(0);
    setTabVisible(1, false);
    comments->clear();
    recommended->clear();
}

void WatchNextFeed::setData(
    const QString& videoId,
    const QList<QtTubePlugin::Video>& recommendedVideos,
    const QtTubePlugin::VideoData::Continuations& continuations)
{
    this->videoId = videoId;

    if (continuations.comments.has_value())
    {
        setTabVisible(1, true);
        commentsContinuation = continuations.comments;
        connect(comments, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueComments);
    }

    if (continuations.recommended.has_value())
    {
        recommended->continuationData = continuations.recommended;
        connect(recommended, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueRecommended);
    }

    populateRecommended(recommendedVideos);
}
