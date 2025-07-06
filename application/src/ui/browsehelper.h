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

    void processChannelTabItems(ContinuableListWidget* widget, const QList<QtTube::ChannelTabDataItem>& items);
private slots:
    void browseFailed(const QString& title, ContinuableListWidget* widget, const QtTube::PluginException& ex);
private:
    QList<std::pair<QString, int>> getActiveFilters(QHBoxLayout* additionalWidgets);
    void setupBrowse(ContinuableListWidget* widget, QtTube::BrowseReply* reply, const QtTube::BrowseData& data);
    void setupChannel(
        ContinuableListWidget* widget, int activeTabIndex,
        QtTube::ChannelReply* reply, const QtTube::ChannelData& data);
    void setupNotifications(
        ContinuableListWidget* widget, QtTube::NotificationsReply* reply, const QtTube::NotificationsData& data);
    void setupSearch(
        ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query,
        const PluginData* plugin, QtTube::BrowseReply* reply, const QtTube::BrowseData& data);
};
