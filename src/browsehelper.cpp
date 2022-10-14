#include "browsehelper.h"
#include "http.h"
#include "settingsstore.h"
#include "ui/browsechannelrenderer.h"
#include "ui/browsevideorenderer.h"
#include <QApplication>
#include <QListWidgetItem>

void BrowseHelper::browseHistory(QListWidget* historyWidget)
{
    try
    {
        if (InnerTube::instance().hasAuthenticated())
        {
            InnertubeEndpoints::BrowseHistory historyData = InnerTube::instance().get<InnertubeEndpoints::BrowseHistory>();
            setupVideoList(historyData.videos, historyWidget);
            continuationToken = historyData.continuationToken;
        }
        else
        {
            historyWidget->addItem("Local history has not been implemented yet. You will need to log in.");
        }
    }
    catch (const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get history browsing data", ie.message());
        else
            qDebug() << "Failed to get history browsing data:" << ie.message();
    }
}

void BrowseHelper::browseHome(QListWidget* homeWidget)
{
    try
    {
        if (SettingsStore::instance().homeShelves)
        {
            const QString clientNameTemp = InnerTube::instance().context()->client.clientName;
            const QString clientVerTemp = InnerTube::instance().context()->client.clientVersion;

            InnerTube::instance().context()->client.clientName = "ANDROID";
            InnerTube::instance().context()->client.clientVersion = "15.14.33";

            InnertubeEndpoints::BrowseHomeShelves homeData = InnerTube::instance().get<InnertubeEndpoints::BrowseHomeShelves>();
            setupVideoList(homeData.videos, homeWidget);
            continuationToken = homeData.continuationToken;

            InnerTube::instance().context()->client.clientName = clientNameTemp;
            InnerTube::instance().context()->client.clientVersion = clientVerTemp;
        }
        else
        {
            InnertubeEndpoints::BrowseHome homeData = InnerTube::instance().get<InnertubeEndpoints::BrowseHome>();
            setupVideoList(homeData.videos, homeWidget);
            continuationToken = homeData.continuationToken;
        }
    }
    catch (const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get home browsing data", ie.message());
        else
            qDebug() << "Failed to get home browsing data:" << ie.message();
    }
}

void BrowseHelper::browseSubscriptions(QListWidget* subsWidget)
{
    try
    {
        if (!InnerTube::instance().hasAuthenticated())
        {
            subsWidget->addItem("You need to log in to view subscriptions.");
            return;
        }

        InnertubeEndpoints::BrowseSubscriptions subsData = InnerTube::instance().get<InnertubeEndpoints::BrowseSubscriptions>();
        setupVideoList(subsData.videos, subsWidget);
        continuationToken = subsData.continuationToken;
    }
    catch (const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get subscriptions browsing data", ie.message());
        else
            qDebug() << "Failed to get subscriptions browsing data:" << ie.message();
    }
}

void BrowseHelper::search(QListWidget* searchWidget, const QString& query)
{
    try
    {
        InnertubeEndpoints::Search searchData = InnerTube::instance().get<InnertubeEndpoints::Search>(query);
        searchWidget->addItem(QStringLiteral("About %1 results").arg(QLocale::system().toString(searchData.estimatedResults)));
        setupChannelList(searchData.channels, searchWidget);
        setupVideoList(searchData.videos, searchWidget);
        continuationToken = searchData.continuationToken;
    }
    catch (const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get search data", ie.message());
        else
            qDebug() << "Failed to get search data:" << ie.message();
    }
}

void BrowseHelper::setupChannelList(const QList<InnertubeObjects::Channel>& channels, QListWidget* widget)
{
    for (const InnertubeObjects::Channel& channel : channels)
    {
        BrowseChannelRenderer* renderer = new BrowseChannelRenderer;
        renderer->setData(channel.channelId, channel.descriptionSnippet.text, channel.title.text, channel.subscribed,
                          channel.subscriberCountText.text, channel.videoCountText.text);

        QListWidgetItem* item = new QListWidgetItem(widget);
        item->setSizeHint(renderer->sizeHint());
        widget->addItem(item);
        widget->setItemWidget(item, renderer);

        HttpReply* reply = Http::instance().get(channel.thumbnails.last().url);
        QObject::connect(reply, &HttpReply::finished, renderer, &BrowseChannelRenderer::setThumbnail);
    }
}

void BrowseHelper::setupVideoList(const QList<InnertubeObjects::Video>& videos, QListWidget* widget)
{
    QSet<QString> processedShelves;
    for (const InnertubeObjects::Video& video : videos)
    {
        if (!video.shelf.text.isEmpty() && !processedShelves.contains(video.shelf.text))
        {
            QLabel* shelfLabel = new QLabel;
            shelfLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 2));
            shelfLabel->setText(video.shelf.text);

            QListWidgetItem* item = new QListWidgetItem(widget);
            item->setSizeHint(shelfLabel->sizeHint());
            widget->addItem(item);
            widget->setItemWidget(item, shelfLabel);

            processedShelves.insert(video.shelf.text);
        }

        BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
        renderer->setChannelData(video.owner);
        renderer->setVideoData(video.lengthText.text, video.publishedTimeText.text, video.startTimeSeconds, video.title.text, video.videoId,
                               video.viewCountText.text);

        QListWidgetItem* item = new QListWidgetItem(widget);
        item->setSizeHint(renderer->sizeHint());
        widget->addItem(item);
        widget->setItemWidget(item, renderer);

        HttpReply* reply = Http::instance().get(video.thumbnail.mqdefault);
        QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
    }
}
