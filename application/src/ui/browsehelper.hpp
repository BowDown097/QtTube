#pragma once
#include "plugins/pluginentry.hpp"
#include "ui/widgets/continuablelistwidget.hpp"
#include <qttube-plugin/components/replytypes/replytypes.h>

class QHBoxLayout;

class BrowseHelper : public QObject
{
    Q_OBJECT
public:
    static BrowseHelper* instance() { static BrowseHelper _instance; return &_instance; }
    explicit BrowseHelper(QObject* parent = nullptr) : QObject(parent) {}

    void browseChannel(
        PluginEntry* plugin, ContinuableListWidget* widget, int activeTabIndex,
        const QString& channelId, const std::any& requestData);
    void browseHistory(PluginEntry* plugin, ContinuableListWidget* widget, const QString& query = "");
    void browseHome(PluginEntry* plugin, ContinuableListWidget* widget);
    void browseNotificationMenu(PluginEntry* plugin, ContinuableListWidget* widget);
    void browseSubscriptions(PluginEntry* plugin, ContinuableListWidget* widget);
    void browseTrending(PluginEntry* plugin, ContinuableListWidget* widget);
    void search(
        PluginEntry* plugin, ContinuableListWidget* widget,
        QHBoxLayout* additionalWidgets, const QString& query);

    void processChannelTabItems(
        ContinuableListWidget* widget, PluginEntry* plugin,
        const QList<QtTubePlugin::ChannelTabDataItem>& items);
private slots:
    void browseFailed(const QString& title, ContinuableListWidget* widget, const QtTubePlugin::Exception& ex);
private:
    std::unordered_map<QString, int> getActiveFilters(QHBoxLayout* additionalWidgets);
    void setupBrowse(
        ContinuableListWidget* widget, PluginEntry* plugin,
        QtTubePlugin::BrowseReply* reply, const QtTubePlugin::BrowseData& data);
    void setupChannel(
        ContinuableListWidget* widget, int activeTabIndex, PluginEntry* plugin,
        QtTubePlugin::ChannelReply* reply, const QtTubePlugin::ChannelData& data);
    void setupNotifications(
        ContinuableListWidget* widget, QtTubePlugin::NotificationsReply* reply, const QtTubePlugin::NotificationsData& data);
    void setupSearch(
        ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query,
        PluginEntry* plugin, QtTubePlugin::BrowseReply* reply, const QtTubePlugin::BrowseData& data);
};
