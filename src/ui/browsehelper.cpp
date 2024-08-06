#include "browsehelper.h"
#include "channelbrowser.h"
#include "mainwindow.h"
#include "protobuf/protobufcompiler.h"
#include "qttubeapplication.h"
#include <ranges>

using namespace InnertubeEndpoints;
using namespace std::placeholders;

BrowseHelper* BrowseHelper::instance()
{
    std::call_once(m_onceFlag, [] { m_instance = new BrowseHelper; });
    return m_instance;
}

void BrowseHelper::browseChannel(QListWidget* widget, int index, const InnertubeEndpoints::ChannelResponse& resp)
{
    QJsonValue tabRenderer = resp.contents["twoColumnBrowseResultsRenderer"]["tabs"][index]["tabRenderer"];
    if (!tabRenderer["selected"].toBool())
    {
        QString params = tabRenderer["endpoint"]["browseEndpoint"]["params"].toString();
        auto bc = InnerTube::instance()->getBlocking<BrowseChannel>(resp.metadata.externalId, "", params);
        tabRenderer = bc.response.contents["twoColumnBrowseResultsRenderer"]["tabs"][index]["tabRenderer"];
    }

    try
    {
        QString title = tabRenderer["title"].toString();
        if (title == "Home")
            ChannelBrowser::setupHome(widget, tabRenderer, resp);
        else if (title == "Videos")
            ChannelBrowser::setupVideos(widget, tabRenderer, resp);
        else if (title == "Shorts")
            ChannelBrowser::setupShorts(widget, tabRenderer, resp);
        else if (title == "Live")
            ChannelBrowser::setupLive(widget, tabRenderer, resp);
        else if (title == "Membership")
            ChannelBrowser::setupMembership(widget, tabRenderer);
        else if (title == "Community")
            ChannelBrowser::setupCommunity(widget, tabRenderer);
        else if (title == "Channels")
            ChannelBrowser::setupChannels(widget, tabRenderer);
        else if (title == "About")
            ChannelBrowser::setupAbout(widget, tabRenderer);
        else
            ChannelBrowser::setupUnimplemented(widget);
    }
    catch (const InnertubeException& ie)
    {
        browseFailed(ie, "channel tab");
    }
}

void BrowseHelper::browseHistory(ContinuableListWidget* widget, const QString& query)
{
    if (!InnerTube::instance()->hasAuthenticated())
    {
        widget->addItem("Local history has not been implemented yet. You will need to log in.");
        return;
    }

    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<BrowseHistory>(query);
    connect(reply, &InnertubeReply<BrowseHistory>::exception, this,
        std::bind(&BrowseHelper::browseFailed, this, _1, "history", widget));
    connect(reply, &InnertubeReply<BrowseHistory>::finished, this, [this, widget](const BrowseHistory& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.videos);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseHome(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<BrowseHome>();
    connect(reply, &InnertubeReply<BrowseHome>::exception, this,
        std::bind(&BrowseHelper::browseFailed, this, _1, "home", widget));
    connect(reply, &InnertubeReply<BrowseHome>::finished, this, [this, widget](const BrowseHome& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.videos);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseNotificationMenu(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<GetNotificationMenu>("NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX");
    connect(reply, &InnertubeReply<GetNotificationMenu>::exception, this,
        std::bind(&BrowseHelper::browseFailed, this, _1, "notification", widget));
    connect(reply, &InnertubeReply<GetNotificationMenu>::finished, this, [this, widget](const GetNotificationMenu& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.notifications);
        widget->continuationToken = endpoint.continuationToken;
        MainWindow::topbar()->updateNotificationCount(0);
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseSubscriptions(ContinuableListWidget* widget)
{
    if (!InnerTube::instance()->hasAuthenticated())
    {
        widget->addItem("You need to log in to view subscriptions.");
        return;
    }

    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<BrowseSubscriptions>();
    connect(reply, &InnertubeReply<BrowseSubscriptions>::exception, this,
        std::bind(&BrowseHelper::browseFailed, this, _1, "subscriptions", widget));
    connect(reply, &InnertubeReply<BrowseSubscriptions>::finished, this, [this, widget](const BrowseSubscriptions& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.videos);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseTrending(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<BrowseTrending>();
    connect(reply, &InnertubeReply<BrowseTrending>::exception, this,
        std::bind(&BrowseHelper::browseFailed, this, _1, "trending", widget));
    connect(reply, &InnertubeReply<BrowseTrending>::finished, this, [this, widget](const BrowseTrending& endpoint) {
        setupTrending(widget, endpoint.response);
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::search(ContinuableListWidget* widget, const QString& query,
                          int dateF, int typeF, int durF, int featF, int sort)
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
        compiledParams = QByteArray::fromHex(ProtobufCompiler::compile(params, searchMsgFields)).toBase64().toPercentEncoding();

    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<Search>(query, "", compiledParams);
    connect(reply, &InnertubeReply<Search>::exception, this,
        std::bind(&BrowseHelper::browseFailed, this, _1, "search", widget));
    connect(reply, &InnertubeReply<Search>::finished, this, [this, widget](const Search& endpoint) {
        widget->addItem(QStringLiteral("About %1 results").arg(QLocale::system().toString(endpoint.response.estimatedResults)));
        setupSearch(widget, endpoint.response);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseFailed(const InnertubeException& ie, const QString& title, ContinuableListWidget* widget)
{
    if (widget)
        widget->setPopulatingFlag(false);

    if (ie.severity() == InnertubeException::Severity::Normal)
        QMessageBox::critical(nullptr, "Failed to get " + title + " data", ie.message());
    else
        qWarning().nospace() << "Failed to get " << title << " data: " << ie.message();
}

void BrowseHelper::removeTrailingSeparator(QListWidget* list)
{
    if (QListWidgetItem* item = list->item(list->count()))
        if (QWidget* itemWidget = list->itemWidget(item))
            if (qobject_cast<QFrame*>(itemWidget))
                itemWidget->deleteLater();
}

void BrowseHelper::setupSearch(QListWidget* widget, const InnertubeEndpoints::SearchResponse& response)
{
    for (const auto& item : response.contents)
    {
        if (const InnertubeObjects::Channel* channel = std::get_if<InnertubeObjects::Channel>(&item))
        {
            UIUtils::addChannelRendererToList(widget, *channel);
            QCoreApplication::processEvents();
        }
        else if (const InnertubeObjects::ReelShelf* reelShelf = std::get_if<InnertubeObjects::ReelShelf>(&item))
        {
            if (qtTubeApp->settings().hideSearchShelves || qtTubeApp->settings().hideShorts)
                continue;

            // TODO: make reel shelf widget, and replace this code for that
            UIUtils::addSeparatorToList(widget);
            UIUtils::addShelfTitleToList(widget, reelShelf->title.text);
            UIUtils::addRangeToList(widget, reelShelf->items);
            UIUtils::addSeparatorToList(widget);
        }
        else if (const InnertubeObjects::VerticalVideoShelf* shelf = std::get_if<InnertubeObjects::VerticalVideoShelf>(&item))
        {
            if (qtTubeApp->settings().hideSearchShelves)
                continue;

            // TODO: make expandable list widget, and replace this code for that
            UIUtils::addSeparatorToList(widget);
            UIUtils::addShelfTitleToList(widget, shelf->title.text);
            UIUtils::addRangeToList(widget, shelf->content.items | std::views::take(shelf->content.collapsedItemCount));
            UIUtils::addSeparatorToList(widget);
        }
        else if (const InnertubeObjects::Video* video = std::get_if<InnertubeObjects::Video>(&item))
        {
            UIUtils::addVideoRendererToList(widget, *video);
            QCoreApplication::processEvents();
        }
    }
}

void BrowseHelper::setupTrending(QListWidget* widget, const InnertubeEndpoints::TrendingResponse& response)
{
    for (const auto& item : response.contents)
    {
        // see above TODO comments
        if (const InnertubeObjects::HorizontalVideoShelf* hvs = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
        {
            UIUtils::addShelfTitleToList(widget, hvs->title.text);
            UIUtils::addRangeToList(widget, hvs->content.items);
            UIUtils::addSeparatorToList(widget);
        }
        else if (const InnertubeObjects::ReelShelf* rs = std::get_if<InnertubeObjects::ReelShelf>(&item))
        {
            if (qtTubeApp->settings().hideShorts)
                continue;

            UIUtils::addShelfTitleToList(widget, rs->title.text);
            UIUtils::addRangeToList(widget, rs->items);
            UIUtils::addSeparatorToList(widget);
        }
        else if (const InnertubeObjects::StandardVideoShelf* svs = std::get_if<InnertubeObjects::StandardVideoShelf>(&item))
        {
            UIUtils::addShelfTitleToList(widget, svs->title.text);
            UIUtils::addRangeToList(widget, svs->content);
            UIUtils::addSeparatorToList(widget);
        }

        removeTrailingSeparator(widget);
    }
}
