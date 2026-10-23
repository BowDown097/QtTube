#pragma once
#include "browsertabwidget.hpp"
#include "plugins/pluginentry.hpp"
#include <QBoxLayout>

class MainWindow;

class MainTabWidget final : public BrowserTabWidget
{
    Q_OBJECT
public:
    explicit MainTabWidget(QWidget* parent = nullptr);
    void activePluginChanged(PluginEntry* activePlugin);
    void browse() override;
    void reloadCurrentTab(ContinuableListWidget* list) override;
    void search(const QString& query);
    void setTopLayout(QHBoxLayout* topLayout);
private:
    enum Tab
    {
        Tab_Home,
        Tab_Trending,
        Tab_Subscriptions,
        Tab_History,
        Tab_Search,
        Tab_HistorySearch,
        Tab_None
    };

    ContinuableListWidget* m_historyWidget;
    ContinuableListWidget* m_historySearchWidget;
    ContinuableListWidget* m_homeWidget;
    ContinuableListWidget* m_searchWidget;
    ContinuableListWidget* m_subscriptionsWidget;
    ContinuableListWidget* m_trendingWidget;

    int m_lastBrowseTab{};
    QString m_lastSearchQuery;
    MainWindow* m_mainWindow;
    PluginEntry* m_plugin;
    QHBoxLayout* m_topLayout;

    void toggleTabsForActivePlugin();
private slots:
    void returnFromSearch(int newTabIndex);
    void searchWatchHistory();
};
