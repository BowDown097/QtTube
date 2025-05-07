#include "watchnextfeed.h"
#include "continuablelistwidget.h"
#include "innertube/endpoints/video/next.h"
#include "qttubeapplication.h"
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

void WatchNextFeed::setData(const InnertubeEndpoints::Next& endpoint)
{
    if (!endpoint.response.contents.results.commentsSectionContinuation.isEmpty())
    {
        setTabVisible(1, true);
        commentsContinuation = endpoint.response.contents.results.commentsSectionContinuation;
        connect(comments, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueComments);
    }

    if (!endpoint.response.contents.secondaryResults.feedContinuation.isEmpty())
    {
        recommendedContinuation = endpoint.response.contents.secondaryResults.feedContinuation;
        connect(recommended, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueRecommended);
    }

    for (const InnertubeObjects::WatchNextFeedItem& item : endpoint.response.contents.secondaryResults.feed)
    {
        if (std::visit([](auto&& v) { return qtTubeApp->settings().videoIsFiltered(v); }, item))
            continue;

        BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
        renderer->thumbnail->setFixedSize(167, 94);
        renderer->titleLabel->setMaximumLines(2);
        renderer->titleLabel->setWordWrap(true);

        if (const auto* compactVideo = std::get_if<InnertubeObjects::CompactVideo>(&item))
        {
            renderer->setData(*compactVideo);
        }
        else if (const auto* adSlot = std::get_if<InnertubeObjects::AdSlot>(&item))
        {
            std::visit([renderer](auto&& v) {
                renderer->setData(v);
            }, adSlot->fulfillmentContent.fulfilledLayout.renderingContent);
        }

        UIUtils::addWidgetToList(recommended, renderer);
        QCoreApplication::processEvents();
    }
}
