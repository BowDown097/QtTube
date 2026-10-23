#include "maintabwidget.hpp"
#include "mainwindow.hpp"
#include "ui/browsehelper.hpp"
#include "ui/views/viewcontroller.hpp"
#include "ui/widgets/topbar/topbar.hpp"
#include "utils/uiutils.hpp"
#include <QLineEdit>

MainTabWidget::MainTabWidget(QWidget* parent)
    : BrowserTabWidget(parent),
      m_historyWidget(new ContinuableListWidget),
      m_historySearchWidget(new ContinuableListWidget),
      m_homeWidget(new ContinuableListWidget),
      m_searchWidget(new ContinuableListWidget),
      m_subscriptionsWidget(new ContinuableListWidget),
      m_trendingWidget(new ContinuableListWidget),
      m_mainWindow(UIUtils::getMainWindow())
{
    setStyleSheet(QStringLiteral(R"(
        QTabBar::tab::last {
            width: 0;
            height: 0;
            margin: 0;
            padding: 0;
            border: none;
        }
    )"));

    {
        QSignalBlocker blocker(this);
        addTab(m_homeWidget, tr("Home"));
        addTab(m_trendingWidget, tr("Trending"));
        addTab(m_subscriptionsWidget, tr("Subscriptions"));
        addTab(m_historyWidget, tr("History"));
        addTab(m_searchWidget, tr("Search"));
        addTab(m_historySearchWidget, tr("History Search"));
        setCurrentIndex(addTab(new QWidget, QString()));
    }

    CONTINUE_CONNECT(m_historyWidget, browseHistory, m_lastSearchQuery);
    CONTINUE_CONNECT(m_historySearchWidget, browseHistory, m_lastSearchQuery);
    CONTINUE_CONNECT(m_homeWidget, browseHome);
    CONTINUE_CONNECT(m_searchWidget, search, nullptr, m_lastSearchQuery);
    CONTINUE_CONNECT(m_subscriptionsWidget, browseSubscriptions);
    CONTINUE_CONNECT(m_trendingWidget, browseTrending);
}

void MainTabWidget::activePluginChanged(PluginEntry* activePlugin)
{
    m_plugin = activePlugin;
    toggleTabsForActivePlugin();

    setCornerWidget(nullptr, Qt::TopRightCorner);
    UIUtils::clearLayout(m_topLayout);
    setTabVisible(Tab_Search, false);
    setTabVisible(Tab_HistorySearch, false);

    if (currentIndex() != Tab_None && isTabVisible(currentIndex()))
        browse();
}

void MainTabWidget::browse()
{
    m_lastBrowseTab = currentIndex();

    setCornerWidget(nullptr, Qt::TopRightCorner);
    UIUtils::clearLayout(m_topLayout);

    m_historySearchWidget->clear();
    m_historyWidget->clear();
    m_homeWidget->clear();
    m_searchWidget->clear();
    m_subscriptionsWidget->clear();
    m_trendingWidget->clear();

    switch (m_lastBrowseTab)
    {
    case Tab_Home:
        m_homeWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseHome(m_plugin, m_homeWidget);
        break;
    case Tab_Trending:
        m_trendingWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseTrending(m_plugin, m_trendingWidget);
        break;
    case Tab_Subscriptions:
        m_subscriptionsWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseSubscriptions(m_plugin, m_subscriptionsWidget);
        break;
    case Tab_History:
        QLineEdit* historySearch = new QLineEdit(this);
        historySearch->setPlaceholderText("Search history");
        setCornerWidget(historySearch, Qt::TopRightCorner);
        historySearch->show();
        connect(historySearch, &QLineEdit::returnPressed, this, &MainTabWidget::searchWatchHistory);
        BrowseHelper::instance()->browseHistory(m_plugin, m_historyWidget);
        break;
    }
}

void MainTabWidget::reloadCurrentTab(ContinuableListWidget* list)
{
    if (currentIndex() <= Tab_History)
    {
        browse();
    }
    else if (currentIndex() == Tab_Search)
    {
        list->clear();
        BrowseHelper::instance()->search(m_plugin, list, m_topLayout, m_lastSearchQuery);
    }
    else if (currentIndex() == Tab_HistorySearch)
    {
        list->clear();
        BrowseHelper::instance()->browseHistory(m_plugin, list, m_lastSearchQuery);
    }
}

void MainTabWidget::returnFromSearch(int newTabIndex)
{
    setCornerWidget(nullptr, Qt::TopRightCorner);
    UIUtils::clearLayout(m_topLayout);
    disconnect(m_mainWindow->topbar()->logo, &TubeLabel::clicked, this, nullptr);

    {
        QSignalBlocker blocker(this);
        setTabVisible(Tab_Search, false);
        setTabVisible(Tab_HistorySearch, false);
        toggleTabsForActivePlugin();
    }

    setCurrentIndex(newTabIndex);

    m_historySearchWidget->clear();
    m_searchWidget->clear();
}

void MainTabWidget::search(const QString& query)
{
    setCornerWidget(nullptr, Qt::TopRightCorner);
    UIUtils::clearLayout(m_topLayout);
    m_historySearchWidget->clear();
    ViewController::unloadCurrent();

    if (currentIndex() == Tab_Search)
    {
        m_searchWidget->clear();
    }
    else
    {
        QSignalBlocker blocker(this);
        connect(m_mainWindow->topbar()->logo, &TubeLabel::clicked, this,
            [this] { returnFromSearch(m_lastBrowseTab); });
        setTabVisible(Tab_Search, true);
        setTabsVisible(false, Tab_Home, Tab_Trending, Tab_Subscriptions, Tab_History, Tab_HistorySearch);
        setCurrentIndex(Tab_Search);
    }

    m_lastSearchQuery = query;
    BrowseHelper::instance()->search(m_plugin, m_searchWidget, m_topLayout, m_lastSearchQuery);
}

void MainTabWidget::searchWatchHistory()
{
    if (currentIndex() == Tab_HistorySearch)
    {
        m_historySearchWidget->clear();
    }
    else
    {
        QSignalBlocker blocker(this);
        connect(m_mainWindow->topbar()->logo, &TubeLabel::clicked, this,
            [this] { returnFromSearch(Tab_History); });
        setTabVisible(Tab_HistorySearch, true);
        setTabsVisible(false, Tab_Home, Tab_Trending, Tab_Subscriptions, Tab_History, Tab_Search);
        setCurrentIndex(Tab_HistorySearch);
    }

    m_lastSearchQuery = qobject_cast<QLineEdit*>(cornerWidget(Qt::TopRightCorner))->text();
    BrowseHelper::instance()->browseHistory(m_plugin, m_historySearchWidget, m_lastSearchQuery);
}

void MainTabWidget::setTopLayout(QHBoxLayout* topLayout)
{
    m_topLayout = topLayout;
}

void MainTabWidget::toggleTabsForActivePlugin()
{
    QSignalBlocker blocker(this);
    m_mainWindow->topbar()->searchBox->setEnabled(m_plugin && m_plugin->providers.search);
    setTabVisible(Tab_Home, m_plugin && m_plugin->providers.home);
    setTabVisible(Tab_Trending, m_plugin && m_plugin->providers.trending);
    setTabVisible(Tab_Subscriptions, m_plugin && m_plugin->providers.subFeed);
    setTabVisible(Tab_History, m_plugin && m_plugin->providers.history);

    // this is required if we're not on the "main page" of the program otherwise the layout breaks,
    // but i'm doing it even if we are on it, just in case.
    tabBar()->adjustSize();
}