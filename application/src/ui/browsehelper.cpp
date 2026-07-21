#include "browsehelper.hpp"
#include "mainwindow.hpp"
#include "qttubeapplication.hpp"
#include "ui/widgets/topbar/topbar.hpp"
#include "utils/uiutils.hpp"
#include <QBoxLayout>
#include <QComboBox>
#include <qttube-plugin/providers/providertypes.h>

void BrowseHelper::browseChannel(
    PluginEntry* plugin, ContinuableListWidget* widget, int activeTabIndex,
    const QString& channelId, const std::any& requestData)
{
    assert(plugin != nullptr);
    assert(plugin->providers.channel != nullptr);

    widget->setPopulatingFlag(true);
    QtTubePlugin::ChannelReply* reply = plugin->providers.channel->getChannel(
        channelId, requestData, widget->continuationData);
    connect(reply, &QtTubePlugin::ChannelReply::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "Channel Tab", widget));
    connect(reply, &QtTubePlugin::ChannelReply::finished, this,
        std::bind_front(&BrowseHelper::setupChannel, this, widget, activeTabIndex, plugin, reply));
}

void BrowseHelper::browseHistory(PluginEntry* plugin, ContinuableListWidget* widget, const QString& query)
{
    assert(plugin != nullptr);
    assert(plugin->providers.history != nullptr);

    if (!plugin->authenticated())
    {
        widget->addItem("Local history is not yet available. You will need to log in.");
        return;
    }

    widget->setPopulatingFlag(true);
    QtTubePlugin::BrowseReply* reply = plugin->providers.history->getHistory(
        query, widget->continuationData);
    connect(reply, &QtTubePlugin::BrowseReply::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "History", widget));
    connect(reply, &QtTubePlugin::BrowseReply::finished, this,
        std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
}

void BrowseHelper::browseHome(PluginEntry* plugin, ContinuableListWidget* widget)
{
    assert(plugin != nullptr);
    assert(plugin->providers.home != nullptr);

    widget->setPopulatingFlag(true);
    QtTubePlugin::BrowseReply* reply = plugin->providers.home->getHome(widget->continuationData);
    connect(reply, &QtTubePlugin::BrowseReply::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "Home Feed", widget));
    connect(reply, &QtTubePlugin::BrowseReply::finished, this,
        std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
}

void BrowseHelper::browseNotificationMenu(PluginEntry* plugin, ContinuableListWidget* widget)
{
    assert(plugin != nullptr);
    assert(plugin->providers.notifs != nullptr);

    widget->setPopulatingFlag(true);
    QtTubePlugin::NotificationsReply* reply = plugin->providers.notifs->getNotifications(
        widget->continuationData);
    connect(reply, &QtTubePlugin::NotificationsReply::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "Notifications", widget));
    connect(reply, &QtTubePlugin::NotificationsReply::finished, this,
        std::bind_front(&BrowseHelper::setupNotifications, this, widget, reply));
}

void BrowseHelper::browseSubscriptions(PluginEntry* plugin, ContinuableListWidget* widget)
{
    assert(plugin != nullptr);
    assert(plugin->providers.subFeed);

    if (!plugin->authenticated())
    {
        widget->addItem("Local subscriptions are not yet available. You will need to log in.");
        return;
    }

    widget->setPopulatingFlag(true);
    QtTubePlugin::BrowseReply* reply = plugin->providers.subFeed->getSubFeed(widget->continuationData);
    connect(reply, &QtTubePlugin::BrowseReply::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "Subscription Feed", widget));
    connect(reply, &QtTubePlugin::BrowseReply::finished, this,
        std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
}

void BrowseHelper::browseTrending(PluginEntry* plugin, ContinuableListWidget* widget)
{
    assert(plugin != nullptr);
    assert(plugin->providers.trending != nullptr);

    widget->setPopulatingFlag(true);
    QtTubePlugin::BrowseReply* reply = plugin->providers.trending->getTrending(widget->continuationData);
    connect(reply, &QtTubePlugin::BrowseReply::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "Trending Feed", widget));
    connect(reply, &QtTubePlugin::BrowseReply::finished, this,
        std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
}

void BrowseHelper::search(
    PluginEntry* plugin, ContinuableListWidget* widget,
    QHBoxLayout* additionalWidgets, const QString& query)
{
    assert(plugin != nullptr);
    assert(plugin->providers.search != nullptr);

    widget->setPopulatingFlag(true);
    QtTubePlugin::BrowseReply* reply = plugin->providers.search->getSearch(
        query, getActiveFilters(additionalWidgets), widget->continuationData);
    connect(reply, &QtTubePlugin::BrowseReply::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "Search Results", widget));
    connect(reply, &QtTubePlugin::BrowseReply::finished, this,
        std::bind_front(&BrowseHelper::setupSearch, this, widget, additionalWidgets, query, plugin, reply));
}

void BrowseHelper::processChannelTabItems(
    ContinuableListWidget* widget, PluginEntry* plugin,
    const QList<QtTubePlugin::ChannelTabDataItem>& items)
{
    for (const QtTubePlugin::ChannelTabDataItem& item : items)
    {
        if (const auto* channel = std::get_if<QtTubePlugin::Channel>(&item))
        {
            UIUtils::addChannelToList(widget, *channel, plugin);
            QCoreApplication::processEvents();
        }
        else if (const auto* video = std::get_if<QtTubePlugin::Video>(&item))
        {
            UIUtils::addVideoToList(widget, *video, plugin);
            QCoreApplication::processEvents();
        }
        else if (const auto* channelShelf = std::get_if<QtTubePlugin::Shelf<QtTubePlugin::Channel>>(&item))
        {
            UIUtils::addShelfTitleToList(widget, channelShelf->title);

            for (const QtTubePlugin::Channel& channel : channelShelf->contents)
            {
                UIUtils::addChannelToList(widget, channel, plugin);
                QCoreApplication::processEvents();
            }

            if (!channelShelf->isDividerHidden)
                UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* videoShelf = std::get_if<QtTubePlugin::Shelf<QtTubePlugin::Video>>(&item))
        {
            UIUtils::addShelfTitleToList(widget, videoShelf->title);

            for (const QtTubePlugin::Video& video : videoShelf->contents)
            {
                UIUtils::addVideoToList(widget, video, plugin);
                QCoreApplication::processEvents();
            }

            if (!videoShelf->isDividerHidden)
                UIUtils::addSeparatorToList(widget);
        }
    }

    if (widget->count() <= 0)
        widget->addItem("No data found.");
}

void BrowseHelper::browseFailed(const QString& title, ContinuableListWidget* widget, const QtTubePlugin::Exception& ex)
{
    widget->setPopulatingFlag(false);
    if (ex.severity() == QtTubePlugin::Exception::Severity::Normal)
        widget->addItem(ex.message());
    else
        qWarning() << "Failed to Load" << title << ':' << ex.message();
}

std::unordered_map<QString, int> BrowseHelper::getActiveFilters(QHBoxLayout* additionalWidgets)
{
    if (!additionalWidgets || additionalWidgets->count() == 0)
        return {};

    QList<QComboBox*> combos;
    for (int i = 0; i < additionalWidgets->count(); ++i)
        if (QComboBox* combo = qobject_cast<QComboBox*>(additionalWidgets->itemAt(i)->widget()))
            combos.append(combo);

    std::unordered_map<QString, int> activeFilters;
    for (QComboBox* combo : std::as_const(combos))
        if (int index = combo->currentIndex(); index != -1)
            activeFilters.emplace(combo->placeholderText(), index);

    return activeFilters;
}

void BrowseHelper::setupBrowse(
    ContinuableListWidget* widget, PluginEntry* plugin,
    QtTubePlugin::BrowseReply* reply, const QtTubePlugin::BrowseData& data)
{
    for (const QtTubePlugin::BrowseDataItem& item : data)
    {
        if (const auto* channel = std::get_if<QtTubePlugin::Channel>(&item))
        {
            UIUtils::addChannelToList(widget, *channel, plugin);
            QCoreApplication::processEvents();
        }
        else if (const auto* shelf = std::get_if<QtTubePlugin::Shelf<QtTubePlugin::Video>>(&item))
        {
            UIUtils::addShelfTitleToList(widget, shelf->title);

            for (const QtTubePlugin::Video& video : shelf->contents)
            {
                UIUtils::addVideoToList(widget, video, plugin);
                QCoreApplication::processEvents();
            }

            if (!shelf->isDividerHidden)
                UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* video = std::get_if<QtTubePlugin::Video>(&item))
        {
            UIUtils::addVideoToList(widget, *video, plugin);
        }

        QCoreApplication::processEvents();
    }

    widget->setPopulatingFlag(false);
    widget->continuationData = reply->continuationData;
}

void BrowseHelper::setupChannel(
    ContinuableListWidget* widget, int activeTabIndex, PluginEntry* plugin,
    QtTubePlugin::ChannelReply* reply, const QtTubePlugin::ChannelData& data)
{
    if (activeTabIndex < data.tabs.size())
        processChannelTabItems(widget, plugin, data.tabs[activeTabIndex].items);
    else if (!data.tabs.isEmpty())
        processChannelTabItems(widget, plugin, data.tabs[0].items);

    widget->setPopulatingFlag(false);
    widget->continuationData = reply->continuationData;
}

void BrowseHelper::setupNotifications(
    ContinuableListWidget* widget, QtTubePlugin::NotificationsReply* reply, const QtTubePlugin::NotificationsData& data)
{
    for (const QtTubePlugin::NotificationsDataItem& notification : data)
    {
        UIUtils::addNotificationToList(widget, notification);
        QCoreApplication::processEvents();
    }

    widget->continuationData = reply->continuationData;
    widget->setPopulatingFlag(false);
    UIUtils::getMainWindow()->topbar()->updateNotificationCount(qtTubeApp->plugins().updatablePlugins().size());
}

void BrowseHelper::setupSearch(
    ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query,
    PluginEntry* plugin, QtTubePlugin::BrowseReply* reply, const QtTubePlugin::BrowseData& data)
{
    if (additionalWidgets && additionalWidgets->count() == 0)
    {
        std::vector<std::pair<QString, QStringList>> filters = plugin->providers.search->searchFilters();
        if (!filters.empty())
        {
            TubeLabel* filtersLabel = new TubeLabel("Filters:");
            additionalWidgets->addWidget(filtersLabel);

            for (const auto& [category, filters] : filters)
            {
                QComboBox* filterCombo = new QComboBox;
                filterCombo->setPlaceholderText(category);
                filterCombo->addItems(filters);
                additionalWidgets->addWidget(filterCombo);

                connect(filterCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this] {
                    widget->clear();
                    search(plugin, widget, additionalWidgets, query);
                });
            }
        }
    }

    setupBrowse(widget, plugin, reply, data);
}
