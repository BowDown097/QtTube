#include "browsehelper.h"
#include "channelbrowser.h"
#include "innertube.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QComboBox>

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
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::BrowseReply* reply = plugin->interface->getHistory(query, widget->continuationToken);
        connect(reply, &QtTube::BrowseReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "history", widget));
        connect(reply, &QtTube::BrowseReply::finished, this,
            std::bind_front(&BrowseHelper::setupBrowse, this, widget, reply));
    }
}

void BrowseHelper::browseHome(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::BrowseReply* reply = plugin->interface->getHome(widget->continuationToken);
        connect(reply, &QtTube::BrowseReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "home", widget));
        connect(reply, &QtTube::BrowseReply::finished, this,
            std::bind_front(&BrowseHelper::setupBrowse, this, widget, reply));
    }
}

void BrowseHelper::browseNotificationMenu(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::NotificationsReply* reply = plugin->interface->getNotifications(widget->continuationToken);
        connect(reply, &QtTube::NotificationsReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "notification", widget));
        connect(reply, &QtTube::NotificationsReply::finished, this,
            std::bind_front(&BrowseHelper::setupNotifications, this, widget, reply));
    }
}

void BrowseHelper::browseSubscriptions(ContinuableListWidget* widget)
{
    if (!InnerTube::instance()->hasAuthenticated())
    {
        widget->addItem("You need to log in to view subscriptions.");
        return;
    }

    widget->setPopulatingFlag(true);
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::BrowseReply* reply = plugin->interface->getSubFeed(widget->continuationToken);
        connect(reply, &QtTube::BrowseReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "subscriptions", widget));
        connect(reply, &QtTube::BrowseReply::finished, this,
            std::bind_front(&BrowseHelper::setupBrowse, this, widget, reply));
    }
}

void BrowseHelper::browseTrending(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::BrowseReply* reply = plugin->interface->getTrending(widget->continuationToken);
        connect(reply, &QtTube::BrowseReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "trending", widget));
        connect(reply, &QtTube::BrowseReply::finished, this,
            std::bind_front(&BrowseHelper::setupBrowse, this, widget, reply));
    }
}

void BrowseHelper::continueChannel(ContinuableListWidget* widget, const QString& channelId)
{
    widget->setPopulatingFlag(true);
    InnertubeReply<BrowseChannel>* reply = InnerTube::instance()->get<BrowseChannel>(channelId, widget->continuationToken);
    connect(reply, &InnertubeReply<BrowseChannel>::exception, this,
        std::bind_front(&BrowseHelper::browseFailedInnertube, this, "channel", widget));
    connect(reply, &InnertubeReply<BrowseChannel>::finished, this, [widget](const BrowseChannel& endpoint) {
        ChannelBrowser::continuation(widget, endpoint.response.contents);
        widget->setPopulatingFlag(false);
    });
}

QList<std::pair<QString, int>> BrowseHelper::getActiveFilters(QHBoxLayout* additionalWidgets)
{
    if (!additionalWidgets || additionalWidgets->count() == 0)
        return {};

    QList<QComboBox*> combos;
    for (int i = 0; i < additionalWidgets->count(); ++i)
        if (QComboBox* combo = qobject_cast<QComboBox*>(additionalWidgets->itemAt(i)->widget()))
            combos.append(combo);

    QList<std::pair<QString, int>> activeFilters;
    for (QComboBox* combo : std::as_const(combos))
        if (int index = combo->currentIndex(); index != -1)
            activeFilters.append(std::make_pair(combo->placeholderText(), index));

    return activeFilters;
}

void BrowseHelper::search(ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query)
{
    widget->setPopulatingFlag(true);
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::BrowseReply* reply = plugin->interface->getSearch(
            query, getActiveFilters(additionalWidgets), widget->continuationToken);
        connect(reply, &QtTube::BrowseReply::exception, this,
            std::bind_front(&BrowseHelper::browseFailedPlugin, this, "search", widget));
        connect(reply, &QtTube::BrowseReply::finished, this,
            std::bind_front(&BrowseHelper::setupSearch, this, widget, additionalWidgets, query, plugin, reply));
    }
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

void BrowseHelper::setupBrowse(ContinuableListWidget* widget, QtTube::BrowseReply* reply, const QtTube::BrowseData& data)
{
    for (const QtTube::BrowseDataItem& item : data)
    {
        if (const auto* channel = std::get_if<QtTube::PluginChannel>(&item))
        {
            UIUtils::addChannelToList(widget, *channel);
            QCoreApplication::processEvents();
        }
        else if (const auto* shelf = std::get_if<QtTube::PluginShelf<QtTube::PluginVideo>>(&item))
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

void BrowseHelper::setupNotifications(
    ContinuableListWidget* widget, QtTube::NotificationsReply* reply, const QtTube::NotificationsData& data)
{
    for (const QtTube::NotificationsDataItem& notification : data)
    {
        UIUtils::addNotificationToList(widget, notification);
        QCoreApplication::processEvents();
    }

    widget->continuationToken = std::any_cast<QString>(reply->continuationData);
    MainWindow::topbar()->updateNotificationCount(0);
    widget->setPopulatingFlag(false);
}

void BrowseHelper::setupSearch(
    ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query,
    const PluginData* plugin, QtTube::BrowseReply* reply, const QtTube::BrowseData& data)
{
    if (additionalWidgets && additionalWidgets->count() == 0)
    {
        const QList<std::pair<QString, QStringList>> searchFilters = plugin->interface->searchFilters();
        if (!searchFilters.empty())
        {
            TubeLabel* filtersLabel = new TubeLabel("Filters:");
            additionalWidgets->addWidget(filtersLabel);

            for (const auto& [category, filters] : searchFilters)
            {
                QComboBox* filterCombo = new QComboBox;
                filterCombo->setPlaceholderText(category);
                filterCombo->addItems(filters);
                additionalWidgets->addWidget(filterCombo);

                connect(filterCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this] {
                    widget->clear();
                    search(widget, additionalWidgets, query);
                });
            }
        }
    }

    setupBrowse(widget, reply, data);
}
