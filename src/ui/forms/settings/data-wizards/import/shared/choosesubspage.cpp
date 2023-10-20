#include "choosesubspage.h"
#include "innertube.h"

constexpr const char* subtitle = "Check the channels you wish to subscribe to, then press Start.";

ChooseSubsPage::ChooseSubsPage(const QList<Entity>& subs, int conclusionPage, const QString& watchHistoryKey,
                               int watchHistoryPage, QWidget* parent)
    : ChooseEntitiesPage(subs, "Choose Subscriptions", subtitle, "Subscribe", "Channel Name", parent),
      conclusionPage(conclusionPage),
      watchHistoryKey(watchHistoryKey),
      watchHistoryPage(watchHistoryPage)
{
    connect(this, &ChooseEntitiesPage::foundEntity, this, &ChooseSubsPage::subToChannelInThread);
}

void ChooseSubsPage::subToChannelInThread(const Entity& channel)
{
    threadPool->start([this, channel] {
        if (!stopped)
        {
            InnerTube::instance().subscribeBlocking(QStringList { channel.id }, true);
            // preventing rate limit.
            // apparently one exists but i didn't hit it when subbing to 100 channels lol.
            // but better safe than sorry!
            QThread::sleep(1);
        }
        emit progress();
    });
}
