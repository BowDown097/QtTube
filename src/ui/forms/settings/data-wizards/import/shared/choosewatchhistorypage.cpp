#include "choosewatchhistorypage.h"
#include "innertube.h"
#include "utils/statsutils.h"
#include <QDebug>

constexpr const char* subtitle = "Check the videos you wish to add to your watch history, then press Start.";

ChooseWatchHistoryPage::ChooseWatchHistoryPage(const QList<Entity>& videos, int conclusionPage, QWidget* parent)
    : ChooseEntitiesPage(videos, "Choose Videos", subtitle, "Add", "Video", parent),
      conclusionPage(conclusionPage)
{
    connect(this, &ChooseEntitiesPage::foundEntity, this, &ChooseWatchHistoryPage::addToWatchHistoryInThread);
}

void ChooseWatchHistoryPage::addToWatchHistoryInThread(const Entity& video)
{
    threadPool->start([this, video] {
        if (!stopped)
        {
            try
            {
                auto player = InnerTube::instance()->getBlocking<InnertubeEndpoints::Player>(video.id);
                StatsUtils::reportPlayback(player.response);
                // prevent rate limit (apparently it exists but i didn't hit it.. better safe than sorry)
                QThread::sleep(1);
            }
            catch (const InnertubeException& ie)
            {
                qWarning().noquote().nospace() <<
                    "InnertubeException importing \"" << video.name << "\": " << ie.message();
            }
        }

        emit progress();
    });
}
