#include "browsehelper.h"
#include "http.h"
#include "protobuf/simpleprotobuf.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/widgets/browsechannelrenderer.h"
#include "ui/widgets/browsenotificationrenderer.h"
#include "ui/widgets/browsevideorenderer.h"
#include <QApplication>
#include <QListWidgetItem>

void BrowseHelper::browseHistory(QListWidget* historyWidget, const QString& query)
{
    if (InnerTube::instance().hasAuthenticated())
    {
        InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseHistory>(query);
        QObject::connect(reply, &InnertubeReply::exception, [](const InnertubeException& ie)
        {
            if (ie.severity() == InnertubeException::Normal)
                QMessageBox::critical(nullptr, "Failed to get history browsing data", ie.message());
            else
                qDebug() << "Failed to get history browsing data:" << ie.message();
        });
        QObject::connect(reply, qOverload<InnertubeEndpoints::BrowseHistory>(&InnertubeReply::finished),
                [this, historyWidget](const InnertubeEndpoints::BrowseHistory& endpoint)
        {
            setupVideoList(endpoint.response.videos, historyWidget);
            continuationToken = endpoint.continuationToken;
        });
    }
    else
    {
        historyWidget->addItem("Local history has not been implemented yet. You will need to log in.");
    }
}

void BrowseHelper::browseHome(QListWidget* homeWidget)
{
    if (SettingsStore::instance().homeShelves)
    {
        const QString clientNameTemp = InnerTube::instance().context()->client.clientName;
        const QString clientVerTemp = InnerTube::instance().context()->client.clientVersion;

        InnerTube::instance().context()->client.clientName = "ANDROID";
        InnerTube::instance().context()->client.clientVersion = "15.14.33";

        InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseHomeShelves>();
        QObject::connect(reply, &InnertubeReply::exception, [](const InnertubeException& ie)
        {
            if (ie.severity() == InnertubeException::Normal)
                QMessageBox::critical(nullptr, "Failed to get home browsing data", ie.message());
            else
                qDebug() << "Failed to get home browsing data:" << ie.message();
        });
        QObject::connect(reply, qOverload<InnertubeEndpoints::BrowseHomeShelves>(&InnertubeReply::finished),
                [this, clientNameTemp, clientVerTemp, homeWidget](const InnertubeEndpoints::BrowseHomeShelves& endpoint)
        {
            setupVideoList(endpoint.response.videos, homeWidget);
            continuationToken = endpoint.continuationToken;
            InnerTube::instance().context()->client.clientName = clientNameTemp;
            InnerTube::instance().context()->client.clientVersion = clientVerTemp;
        });
    }
    else
    {
        InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseHome>();
        QObject::connect(reply, &InnertubeReply::exception, [](const InnertubeException& ie)
        {
            if (ie.severity() == InnertubeException::Normal)
                QMessageBox::critical(nullptr, "Failed to get home browsing data", ie.message());
            else
                qDebug() << "Failed to get home browsing data:" << ie.message();
        });
        QObject::connect(reply, qOverload<InnertubeEndpoints::BrowseHome>(&InnertubeReply::finished),
                [this, homeWidget](const InnertubeEndpoints::BrowseHome& endpoint)
        {
            setupVideoList(endpoint.response.videos, homeWidget);
            continuationToken = endpoint.continuationToken;
        });
    }
}

// ok listen i know i'm kind of cheating here but whatever
void BrowseHelper::browseNotificationMenu(QListWidget* menuWidget)
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::GetNotificationMenu>("NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX");
    QObject::connect(reply, &InnertubeReply::exception, [](const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get notification data", ie.message());
        else
            qDebug() << "Failed to get notification data:" << ie.message();
    });
    QObject::connect(reply, qOverload<InnertubeEndpoints::GetNotificationMenu>(&InnertubeReply::finished),
            [this, menuWidget](const InnertubeEndpoints::GetNotificationMenu& endpoint)
    {
        setupNotificationList(endpoint.response.notifications, menuWidget);
        continuationToken = endpoint.continuationToken;
        MainWindow::topbar()->updateNotificationCount();
    });
}

void BrowseHelper::browseSubscriptions(QListWidget* subsWidget)
{
    if (!InnerTube::instance().hasAuthenticated())
    {
        subsWidget->addItem("You need to log in to view subscriptions.");
        return;
    }

    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseSubscriptions>();
    QObject::connect(reply, &InnertubeReply::exception, [](const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get subscriptions browsing data", ie.message());
        else
            qDebug() << "Failed to get subscriptions browsing data:" << ie.message();
    });
    QObject::connect(reply, qOverload<InnertubeEndpoints::BrowseSubscriptions>(&InnertubeReply::finished),
            [this, subsWidget](const InnertubeEndpoints::BrowseSubscriptions& endpoint)
    {
        setupVideoList(endpoint.response.videos, subsWidget);
        continuationToken = endpoint.continuationToken;
    });
}

void BrowseHelper::browseTrending(QListWidget* trendingWidget)
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseTrending>();
    QObject::connect(reply, &InnertubeReply::exception, [](const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get trending browsing data", ie.message());
        else
            qDebug() << "Failed to get trending browsing data:" << ie.message();
    });
    QObject::connect(reply, qOverload<InnertubeEndpoints::BrowseTrending>(&InnertubeReply::finished),
            [this, trendingWidget](const InnertubeEndpoints::BrowseTrending& endpoint)
    {
        setupVideoList(endpoint.response.videos, trendingWidget);
    });
}

void BrowseHelper::search(QListWidget* searchWidget, const QString& query, int dateF, int typeF, int durF, int featF, int sort)
{
    QByteArray compiledParams;
    QVariantMap filter, params;
    if (sort != -1) params.insert("sort", sort);
    if (dateF != -1) filter.insert("uploadDate", dateF + 1);
    if (typeF != -1) filter.insert("type", typeF + 1);
    if (durF != -1) filter.insert("duration", durF + 1);
    if (featF != -1) filter.insert(featureMap[featF], true);
    if (!filter.isEmpty()) params.insert("filter", filter);

    if (!params.isEmpty())
        compiledParams = QByteArray::fromHex(SimpleProtobuf::compile(params, searchMsgFields)).toBase64().toPercentEncoding();

    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::Search>(query, "", compiledParams);
    QObject::connect(reply, &InnertubeReply::exception, [](const InnertubeException& ie)
    {
        if (ie.severity() == InnertubeException::Normal)
            QMessageBox::critical(nullptr, "Failed to get search data", ie.message());
        else
            qDebug() << "Failed to get search data:" << ie.message();
    });
    QObject::connect(reply, qOverload<InnertubeEndpoints::Search>(&InnertubeReply::finished),
            [this, searchWidget](const InnertubeEndpoints::Search& endpoint)
    {
        searchWidget->addItem(QStringLiteral("About %1 results").arg(QLocale::system().toString(endpoint.response.estimatedResults)));
        setupChannelList(endpoint.response.channels, searchWidget);
        setupVideoList(endpoint.response.videos, searchWidget);
        continuationToken = endpoint.continuationToken;
    });
}

void BrowseHelper::setupChannelList(const QList<InnertubeObjects::SearchChannel>& channels, QListWidget* widget)
{
    for (const InnertubeObjects::SearchChannel& channel : channels)
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

void BrowseHelper::setupNotificationList(const QList<InnertubeObjects::Notification>& notifications, QListWidget* widget)
{
    for (const InnertubeObjects::Notification& n : notifications)
    {
        BrowseNotificationRenderer* renderer = new BrowseNotificationRenderer(widget);
        renderer->setData(n);

        QListWidgetItem* item = new QListWidgetItem(widget);
        item->setSizeHint(renderer->sizeHint());
        widget->addItem(item);
        widget->setItemWidget(item, renderer);

        HttpReply* iconReply = Http::instance().get(n.channelIcon.url);
        QObject::connect(iconReply, &HttpReply::finished, renderer, &BrowseNotificationRenderer::setChannelIcon);

        HttpReply* thumbReply = Http::instance().get("https://i.ytimg.com/vi/" + n.videoId + "/mqdefault.jpg");
        QObject::connect(thumbReply, &HttpReply::finished, renderer, &BrowseNotificationRenderer::setThumbnail);
    }
}

void BrowseHelper::setupVideoList(const QList<InnertubeObjects::Video>& videos, QListWidget* widget)
{
    QString lastShelf;
    for (const InnertubeObjects::Video& video : videos)
    {
        if (!video.shelf.text.isEmpty() && video.shelf.text != lastShelf)
        {
            TubeLabel* shelfLabel = new TubeLabel(video.shelf.text);
            shelfLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));

            QListWidgetItem* item = new QListWidgetItem(widget);
            item->setSizeHint(shelfLabel->sizeHint());
            widget->addItem(item);
            widget->setItemWidget(item, shelfLabel);

            lastShelf = video.shelf.text;
        }

        BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
        renderer->setChannelData(video.owner);
        renderer->setVideoData(video.lengthText.text, video.publishedTimeText.text, video.startTimeSeconds, video.title.text, video.videoId,
                               SettingsStore::instance().condensedViews ? video.shortViewCountText.text : video.viewCountText.text);

        QListWidgetItem* item = new QListWidgetItem(widget);
        item->setSizeHint(renderer->sizeHint());
        widget->addItem(item);
        widget->setItemWidget(item, renderer);

        HttpReply* reply = Http::instance().get(video.thumbnail.mqdefault);
        QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
    }
}
