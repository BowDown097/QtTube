#include "browsehelper.h"
#include "channelbrowser.h"
#include "mainwindow.h"
#include "protobuf/protobufcompiler.h"
#include "qttubeapplication.h"
#include <ranges>

// TODO: incrementally switch everything over to using the plugin interface.
// once all of this stuff is said and done,
// ContinuableListWidget::continuationToken needs to be made a std::any itself
// to support plugins that may not take a token in particular.
// not doing this already for compatibility sake and because the YouTube plugin is the only one.

using namespace InnertubeEndpoints;

void BrowseHelper::browseChannel(ContinuableListWidget* widget, int index, const InnertubeEndpoints::ChannelResponse& resp)
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
        QString commandUrl = tabRenderer["endpoint"]["commandMetadata"]["webCommandMetadata"]["url"].toString();
        if (commandUrl.endsWith("/featured"))
            ChannelBrowser::setupHome(widget, tabRenderer);
        else if (commandUrl.endsWith("/videos"))
            ChannelBrowser::setupVideos(widget, tabRenderer);
        else if (commandUrl.endsWith("/shorts"))
            ChannelBrowser::setupShorts(widget, tabRenderer);
        else if (commandUrl.endsWith("/streams"))
            ChannelBrowser::setupLive(widget, tabRenderer);
        else if (commandUrl.endsWith("/membership"))
            ChannelBrowser::setupMembership(widget, tabRenderer);
        else if (commandUrl.endsWith("/community"))
            ChannelBrowser::setupCommunity(widget, tabRenderer);
        else
            ChannelBrowser::setupUnimplemented(widget);
    }
    catch (const InnertubeException& ie)
    {
        browseFailedInnertube("channel tab", nullptr, ie);
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
        std::bind_front(&BrowseHelper::browseFailedInnertube, this, "history", widget));
    connect(reply, &InnertubeReply<BrowseHistory>::finished, this, [this, widget](const BrowseHistory& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.videos);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseHome(ContinuableListWidget* widget, const QString& continuationToken)
{
    widget->setPopulatingFlag(true);
    for (const PluginData* plugin : qtTubeApp->plugins().plugins())
    {
        QtTube::BrowseReply* reply = plugin->interface->getHome(continuationToken);
        connect(reply, &QtTube::BrowseReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "home", widget));
        connect(reply, &QtTube::BrowseReply::finished, this,
            std::bind_front(&BrowseHelper::setupBrowse, this, widget, reply));
    }
}

void BrowseHelper::browseNotificationMenu(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<GetNotificationMenu>("NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX");
    connect(reply, &InnertubeReply<GetNotificationMenu>::exception, this,
        std::bind_front(&BrowseHelper::browseFailedInnertube, this, "notification", widget));
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
        std::bind_front(&BrowseHelper::browseFailedInnertube, this, "subscriptions", widget));
    connect(reply, &InnertubeReply<BrowseSubscriptions>::finished, this, [this, widget](const BrowseSubscriptions& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.videos);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseTrending(ContinuableListWidget* widget, const QString& continuationToken)
{
    widget->setPopulatingFlag(true);
    for (const PluginData* plugin : qtTubeApp->plugins().plugins())
    {
        QtTube::BrowseReply* reply = plugin->interface->getTrending(continuationToken);
        connect(reply, &QtTube::BrowseReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "trending", widget));
        connect(reply, &QtTube::BrowseReply::finished, this,
            std::bind_front(&BrowseHelper::setupBrowse, this, widget, reply));
    }
}

void BrowseHelper::continueChannel(ContinuableListWidget* widget, const QJsonValue& contents)
{
    widget->continuationToken.clear();
    ChannelBrowser::continuation(widget, contents);
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
        compiledParams = ProtobufCompiler::compileEncoded(params, searchMsgFields);

    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<Search>(query, "", compiledParams);
    connect(reply, &InnertubeReply<Search>::exception, this,
        std::bind_front(&BrowseHelper::browseFailedInnertube, this, "search", widget));
    connect(reply, &InnertubeReply<Search>::finished, this, [this, widget](const Search& endpoint) {
        widget->addItem(QStringLiteral("About %1 results").arg(QLocale::system().toString(endpoint.response.estimatedResults)));
        setupSearch(widget, endpoint.response);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseFailedInnertube(const QString& title, ContinuableListWidget* widget, const InnertubeException& ex)
{
    if (widget)
        widget->setPopulatingFlag(false);

    if (ex.severity() == InnertubeException::Severity::Normal)
        QMessageBox::critical(nullptr, "Failed to get " + title + " data", ex.message());
    else
        qWarning().nospace() << "Failed to get " << title << " data: " << ex.message();
}

void BrowseHelper::browseFailedPlugin(const QString& title, ContinuableListWidget* widget, const QtTube::PluginException& ex)
{
    if (widget)
        widget->setPopulatingFlag(false);

    if (ex.severity() == QtTube::PluginException::Severity::Normal)
        QMessageBox::critical(nullptr, "Failed to get " + title + " data", ex.message());
    else
        qWarning().nospace() << "Failed to get " << title << " data:" << ex.message();
}

void BrowseHelper::removeTrailingSeparator(QListWidget* list)
{
    if (QListWidgetItem* item = list->item(list->count()))
        if (QWidget* itemWidget = list->itemWidget(item))
            if (qobject_cast<QFrame*>(itemWidget))
                itemWidget->deleteLater();
}

void BrowseHelper::setupBrowse(ContinuableListWidget* widget, QtTube::BrowseReply* reply, const QtTube::BrowseData& data)
{
    for (const QtTube::BrowseDataItem& item : data)
    {
        if (const auto* shelf = std::get_if<QtTube::PluginShelf<QtTube::PluginVideo>>(&item))
        {
            UIUtils::addShelfTitleToList(widget, shelf->title);

            for (const QtTube::PluginVideo& video : shelf->contents)
            {
                UIUtils::addVideoToList(widget, video);
                QCoreApplication::processEvents();
            }

            if (!shelf->isDividerHidden)
                UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* video = std::get_if<QtTube::PluginVideo>(&item))
        {
            UIUtils::addVideoToList(widget, *video);
        }

        QCoreApplication::processEvents();
    }

    widget->setPopulatingFlag(false);
    widget->continuationToken = std::any_cast<QString>(reply->continuationData);
}

void BrowseHelper::setupSearch(QListWidget* widget, const InnertubeEndpoints::SearchResponse& response)
{
    for (const InnertubeEndpoints::SearchResponseItem& item : response.contents)
    {
        if (const auto* channel = std::get_if<InnertubeObjects::Channel>(&item))
        {
            UIUtils::addChannelToList(widget, *channel);
            QCoreApplication::processEvents();
        }
        else if (const auto* reelShelf = std::get_if<InnertubeObjects::ReelShelf>(&item))
        {
            if (qtTubeApp->settings().hideSearchShelves || qtTubeApp->settings().hideShorts)
                continue;

            UIUtils::addSeparatorToList(widget);
            UIUtils::addShelfTitleToList(widget, reelShelf->title.text);
            UIUtils::addRangeToList(widget, reelShelf->items);
            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* verticalShelf = std::get_if<InnertubeObjects::VerticalVideoShelf>(&item))
        {
            if (qtTubeApp->settings().hideSearchShelves)
                continue;

            UIUtils::addSeparatorToList(widget);
            UIUtils::addShelfTitleToList(widget, verticalShelf->title.text);
            UIUtils::addRangeToList(widget,
                verticalShelf->content.items | std::views::take(verticalShelf->content.collapsedItemCount));
            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
        {
            UIUtils::addVideoToList(widget, *video);
            QCoreApplication::processEvents();
        }
    }
}
