#include "choosesubspage.h"
#include "innertube.h"

constexpr QLatin1String Subtitle("Check the channels you wish to subscribe to, then press Start.");

ChooseSubsPage::ChooseSubsPage(const QList<Entity>& subs, int conclusionPage, const QString& watchHistoryKey,
                               int watchHistoryPage, QWidget* parent)
    : ChooseEntitiesPage(subs, "Choose Subscriptions", Subtitle, "Subscribe", "Channel Name", parent),
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
            InnerTube::instance()->subscribeBlocking(QStringList { channel.id }, true);
            // prevent rate limit (apparently it exists but i didn't hit it.. better safe than sorry)
            QThread::sleep(1);
        }

        emit progress();
    });
}
