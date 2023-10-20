#include "choosewatchhistorypage.h"

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
        qDebug() << video.id << video.name;
        emit progress();
    });
}
