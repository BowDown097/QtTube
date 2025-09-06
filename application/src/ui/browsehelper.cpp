#include "browsehelper.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QComboBox>

void BrowseHelper::browseChannel(
    ContinuableListWidget* widget, int activeTabIndex,
    const QString& channelId, std::any requestData)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::ChannelReply* reply = plugin->interface->getChannel(
                channelId, requestData, widget->continuationData))
        {
            widget->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::ChannelReply::exception, this,
                    std::bind_front(&BrowseHelper::browseFailed, this, "Channel Tab", widget));
            connect(reply, &QtTubePlugin::ChannelReply::finished, this,
                    std::bind_front(&BrowseHelper::setupChannel, this, widget, activeTabIndex, plugin, reply));
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

void BrowseHelper::browseHistory(ContinuableListWidget* widget, const QString& query)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (!plugin->auth || plugin->auth->isEmpty())
        {
            widget->addItem("Local history is not yet available. You will need to log in.");
            return;
        }

        if (QtTubePlugin::BrowseReply* reply = plugin->interface->getHistory(query, widget->continuationData))
        {
            widget->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::BrowseReply::exception, this,
                std::bind_front(&BrowseHelper::browseFailed, this, "History", widget));
            connect(reply, &QtTubePlugin::BrowseReply::finished, this,
                std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

void BrowseHelper::browseHome(ContinuableListWidget* widget)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::BrowseReply* reply = plugin->interface->getHome(widget->continuationData))
        {
            widget->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::BrowseReply::exception, this,
                std::bind_front(&BrowseHelper::browseFailed, this, "Home Feed", widget));
            connect(reply, &QtTubePlugin::BrowseReply::finished, this,
                std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

void BrowseHelper::browseNotificationMenu(ContinuableListWidget* widget)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::NotificationsReply* reply = plugin->interface->getNotifications(widget->continuationData))
        {
            widget->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::NotificationsReply::exception, this,
                std::bind_front(&BrowseHelper::browseFailed, this, "Notifications", widget));
            connect(reply, &QtTubePlugin::NotificationsReply::finished, this,
                std::bind_front(&BrowseHelper::setupNotifications, this, widget, reply));
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

void BrowseHelper::browseSubscriptions(ContinuableListWidget* widget)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (!plugin->auth || plugin->auth->isEmpty())
        {
            widget->addItem("Local subscriptions are not yet available. You will need to log in.");
            return;
        }

        if (QtTubePlugin::BrowseReply* reply = plugin->interface->getSubFeed(widget->continuationData))
        {
            widget->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::BrowseReply::exception, this,
                std::bind_front(&BrowseHelper::browseFailed, this, "Subscription Feed", widget));
            connect(reply, &QtTubePlugin::BrowseReply::finished, this,
                std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

void BrowseHelper::browseTrending(ContinuableListWidget* widget)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::BrowseReply* reply = plugin->interface->getTrending(widget->continuationData))
        {
            widget->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::BrowseReply::exception, this,
                std::bind_front(&BrowseHelper::browseFailed, this, "Trending Feed", widget));
            connect(reply, &QtTubePlugin::BrowseReply::finished, this,
                std::bind_front(&BrowseHelper::setupBrowse, this, widget, plugin, reply));
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

void BrowseHelper::search(ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::BrowseReply* reply = plugin->interface->getSearch(
                query, getActiveFilters(additionalWidgets), widget->continuationData))
        {
            widget->setPopulatingFlag(true);
            connect(reply, &QtTubePlugin::BrowseReply::exception, this,
                std::bind_front(&BrowseHelper::browseFailed, this, "Search Results", widget));
            connect(reply, &QtTubePlugin::BrowseReply::finished, this,
                std::bind_front(&BrowseHelper::setupSearch, this, widget, additionalWidgets, query, plugin, reply));
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

void BrowseHelper::processChannelTabItems(
    ContinuableListWidget* widget, PluginData* plugin,
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
    if (widget)
        widget->setPopulatingFlag(false);

    if (ex.severity() == QtTubePlugin::Exception::Severity::Normal)
        QMessageBox::critical(nullptr, "Failed to Load " + title, ex.message());
    else
        qWarning() << "Failed to Load" << title << ':' << ex.message();
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

void BrowseHelper::setupBrowse(
    ContinuableListWidget* widget, PluginData* plugin,
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
    ContinuableListWidget* widget, int activeTabIndex, PluginData* plugin,
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
    MainWindow::topbar()->updateNotificationCount(0);
    widget->setPopulatingFlag(false);
}

void BrowseHelper::setupSearch(
    ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query,
    PluginData* plugin, QtTubePlugin::BrowseReply* reply, const QtTubePlugin::BrowseData& data)
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

    setupBrowse(widget, plugin, reply, data);
}
