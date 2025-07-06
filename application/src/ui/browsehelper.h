#pragma once
#include "qttube-plugin/components/replytypes/replytypes.h"
#include "ui/widgets/continuablelistwidget.h"
#include <QMessageBox>
#include <QScrollBar>

struct PluginData;
class QHBoxLayout;

class BrowseHelper : public QObject
{
    Q_OBJECT
public:
    static BrowseHelper* instance() { static BrowseHelper _instance; return &_instance; }
    explicit BrowseHelper(QObject* parent = nullptr) : QObject(parent) {}

    void browseChannel(
        ContinuableListWidget* widget, int activeTabIndex,
        const QString& channelId, std::any requestData);
    void browseHistory(ContinuableListWidget* widget, const QString& query = "");
    void browseHome(ContinuableListWidget* widget);
    void browseNotificationMenu(ContinuableListWidget* widget);
    void browseSubscriptions(ContinuableListWidget* widget);
    void browseTrending(ContinuableListWidget* widget);
    void search(ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query);

    void processChannelTabItems(ContinuableListWidget* widget, const QList<QtTubePlugin::ChannelTabDataItem>& items);
private slots:
    void browseFailed(const QString& title, ContinuableListWidget* widget, const QtTubePlugin::Exception& ex);
private:
    QList<std::pair<QString, int>> getActiveFilters(QHBoxLayout* additionalWidgets);
    void setupBrowse(ContinuableListWidget* widget, QtTubePlugin::BrowseReply* reply, const QtTubePlugin::BrowseData& data);
    void setupChannel(
        ContinuableListWidget* widget, int activeTabIndex,
        QtTubePlugin::ChannelReply* reply, const QtTubePlugin::ChannelData& data);
    void setupNotifications(
        ContinuableListWidget* widget, QtTubePlugin::NotificationsReply* reply, const QtTubePlugin::NotificationsData& data);
    void setupSearch(
        ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query,
        const PluginData* plugin, QtTubePlugin::BrowseReply* reply, const QtTubePlugin::BrowseData& data);
};
