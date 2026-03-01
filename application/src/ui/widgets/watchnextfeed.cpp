#include "watchnextfeed.h"
#include "continuablelistwidget.h"
#include "plugins/pluginentry.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "ui/widgets/renderers/video/videothumbnailwidget.h"
#include "utils/uiutils.h"
#include <QMessageBox>

WatchNextFeed::WatchNextFeed(PluginEntry* plugin, QWidget* parent)
    : QTabWidget(parent),
      m_commentsList(new ContinuableListWidget(this)),
      m_plugin(plugin),
      m_recommendedList(new ContinuableListWidget(this))
{
    addTab(m_recommendedList, "Recommended");
    addTab(m_commentsList, "Comments");
    setTabVisible(1, false);
}

void WatchNextFeed::continueComments()
{
}

void WatchNextFeed::continueRecommended()
{
    if (QtTubePlugin::RecommendedContinuationReply* reply = m_plugin->interface->continueRecommended(
            m_videoId, m_recommendedList->continuationData))
    {
        m_recommendedList->setPopulatingFlag(true);
        connect(reply, &QtTubePlugin::RecommendedContinuationReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
            m_recommendedList->setPopulatingFlag(false);
            QMessageBox::critical(nullptr, "Failed to Load Recommended Content", ex.message());
        });
        connect(reply, &QtTubePlugin::RecommendedContinuationReply::finished,
                this, &WatchNextFeed::continueRecommendedFinished);
    }
}

void WatchNextFeed::continueRecommendedFinished(const QtTubePlugin::RecommendedContinuationData& data)
{
    m_recommendedList->continuationData = data.nextContinuation;
    populateRecommended(data.videos);
    m_recommendedList->setPopulatingFlag(false);
}

ContinuableListWidget* WatchNextFeed::currentList()
{
    return qobject_cast<ContinuableListWidget*>(currentWidget());
}

void WatchNextFeed::populateRecommended(const QList<QtTubePlugin::Video>& videos)
{
    for (const QtTubePlugin::Video& video : videos)
    {
        BrowseVideoRenderer* renderer = new BrowseVideoRenderer(m_plugin);
        renderer->thumbnail->setFixedSize(167, 94);
        renderer->titleLabel->setMaximumLines(2);
        renderer->titleLabel->setWordWrap(true);
        renderer->setData(video);

        UIUtils::addWidgetToList(m_recommendedList, renderer);
        QCoreApplication::processEvents();
    }
}

void WatchNextFeed::reset()
{
    setCurrentIndex(0);
    setTabVisible(1, false);
    m_commentsList->clear();
    m_recommendedList->clear();
}

void WatchNextFeed::setData(
    const QString& videoId,
    const QList<QtTubePlugin::Video>& recommendedVideos,
    const QtTubePlugin::VideoData::Continuations& continuations)
{
    m_videoId = videoId;

    if (continuations.comments.has_value())
    {
        setTabVisible(1, true);
        m_commentsList->continuationData = continuations.comments;
        connect(m_commentsList, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueComments);
    }

    if (continuations.recommended.has_value())
    {
        m_recommendedList->continuationData = continuations.recommended;
        connect(m_recommendedList, &ContinuableListWidget::continuationReady, this, &WatchNextFeed::continueRecommended);
    }

    populateRecommended(recommendedVideos);
}
