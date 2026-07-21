#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "qttubeapplication.hpp"
#include "ui/browsehelper.hpp"
#include "ui/forms/plugins/pluginbrowserview.hpp"
#include "ui/views/viewcontroller.hpp"
#include "ui/widgets/accountmenu/accountcontrollerwidget.hpp"
#include "ui/widgets/findbar.hpp"
#include "ui/widgets/renderers/browsenotificationrenderer.hpp"
#include "ui/widgets/topbar/topbar.hpp"
#include "utils/uiutils.hpp"
#include <QAction>
#include <QLineEdit>
#include <QMessageBox>
#include <qttube-plugin/components/auth/authstore.h>

enum Tabs
{
    TAB_HOME,
    TAB_TRENDING,
    TAB_SUBSCRIPTIONS,
    TAB_HISTORY,
    TAB_SEARCH,
    TAB_HISTORY_SEARCH,
    TAB_NONE
};

MainWindow::~MainWindow() { delete ui; }

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QTTUBE_APP_NAME);

    m_topbar = new TopBar(this);

    m_notificationMenu = new ContinuableListWidget(this);
    m_notificationMenu->hide();
    m_notificationMenu->setContinuationThreshold(5);

    m_findbar = new FindBar(this);
    connect(ui->centralwidget, &QStackedWidget::currentChanged, this, [this] {
        if (m_findbar->isVisible())
            m_findbar->setReveal(false);
    });

    connect(m_topbar->avatarButton, &TubeLabel::clicked, this, &MainWindow::toggleAccountMenu);
    connect(m_topbar->notificationBell, &TopBarBell::clicked, this, &MainWindow::toggleNotificationMenu);
    connect(m_topbar->searchBox, &SearchBox::searchRequested, this, &MainWindow::search);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::browse);

    connect(m_notificationMenu, &ContinuableListWidget::continuationReady, this, [this] {
        if (m_notificationMenu->continuationData.has_value())
            BrowseHelper::instance()->browseNotificationMenu(m_activePlugin, m_notificationMenu);
    });
    connect(ui->historyWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->historyWidget->continuationData.has_value())
            BrowseHelper::instance()->browseHistory(m_activePlugin, ui->historyWidget, m_lastSearchQuery);
    });
    connect(ui->homeWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->homeWidget->continuationData.has_value())
            BrowseHelper::instance()->browseHome(m_activePlugin, ui->homeWidget);
    });
    connect(ui->searchWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->searchWidget->continuationData.has_value())
            BrowseHelper::instance()->search(m_activePlugin, ui->searchWidget, nullptr, m_lastSearchQuery);
    });
    connect(ui->subscriptionsWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->subscriptionsWidget->continuationData.has_value())
            BrowseHelper::instance()->browseSubscriptions(m_activePlugin, ui->subscriptionsWidget);
    });

    QAction* reloadShortcut = new QAction(this);
    reloadShortcut->setAutoRepeat(false);
    reloadShortcut->setShortcuts(QList<QKeySequence>() << Qt::Key_F5 << QKeySequence(Qt::ControlModifier | Qt::Key_R));
    connect(reloadShortcut, &QAction::triggered, this, &MainWindow::reloadCurrentTab);
    addAction(reloadShortcut);

    qtTubeApp->doInitialSetup();
    connect(qtTubeApp, &QtTubeApplication::activePluginChanged, this, &MainWindow::activePluginChanged);
    connect(&qtTubeApp->plugins(), &PluginManager::foundUpdate, this, &MainWindow::pluginUpdateAvailable);

#ifdef Q_OS_LINUX
    if (qtTubeApp->settings().playerSettings.vaapi)
    {
        qputenv("LIBVA_DRI3_DISABLE", "1"); // fixes issue on some older GPUs
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", qgetenv("QTWEBENGINE_CHROMIUM_FLAGS") + " --enable-features=VaapiVideoDecoder --enable-features=VaapiIgnoreDriverChecks --disable-features=UseChromeOSDirectVideoDecoder");
    }
#endif

    QCommandLineParser& parser = qtTubeApp->commandLineParser();
    if (PluginEntry* plugin = qtTubeApp->plugins().activePlugin())
    {
        activePluginChanged(plugin);
        if (parser.isSet("channel"))
            ViewController::loadChannel(parser.value("channel"), plugin);
        else if (parser.isSet("video"))
            ViewController::loadVideo(parser.value("video"), plugin);
    }
    else if (parser.isSet("use-plugin"))
    {
        qCritical() << "Plugin not found.";
        exit(EXIT_FAILURE);
    }
    else if (!qtTubeApp->plugins().hasLoadablePlugins())
    {
        // wrapped in this manner to avoid blocking displaying of the main window
        QMetaObject::invokeMethod(this, [this] {
            if (QMessageBox::question(this, "Browse Plugins?", "You have no plugins installed. Would you like to open the plugin browser?") == QMessageBox::Yes)
                PluginBrowserView::spawn();
        }, Qt::QueuedConnection);
    }
}

void MainWindow::activePluginChanged(PluginEntry* activePlugin)
{
    m_activePlugin = activePlugin;
    toggleTabsForActivePlugin();

    UIUtils::clearLayout(ui->additionalWidgets);
    ui->tabWidget->setTabVisible(TAB_SEARCH, false);
    ui->tabWidget->setTabVisible(TAB_HISTORY_SEARCH, false);

    if (int i = ui->tabWidget->currentIndex(); i != TAB_NONE && ui->tabWidget->isTabVisible(i))
        browse();

    if (QtTubePlugin::AuthStoreBase* authStore = activePlugin->authStore)
    {
        if (const QtTubePlugin::AuthUser* authUser = authStore->activeBaseLogin())
        {
            m_topbar->avatarButton->setImage(authUser->avatar, TubeLabel::Cached | TubeLabel::Rounded);
            m_topbar->postSignInSetup(activePlugin);
        }
        else
        {
            m_topbar->updateUIForSignInState(false);
        }

        connect(authStore, &QtTubePlugin::AuthStoreBase::authenticateSuccess,
                this, &MainWindow::authSucceeded, Qt::UniqueConnection);
        connect(authStore, &QtTubePlugin::AuthStoreBase::updateFail,
                this, &MainWindow::authFailed, Qt::UniqueConnection);
    }
    else
    {
        m_topbar->updateUIForSignInState(false);
    }
}

void MainWindow::authFailed()
{
    QMessageBox::critical(nullptr, "Invalid Login",
        "Your session has expired or your credentials are invalid. "
        "You will be logged out. Try logging in again.");
    m_topbar->signOut(m_activePlugin);
}

void MainWindow::authSucceeded()
{
    m_topbar->postSignInSetup(m_activePlugin);
}

void MainWindow::browse()
{
    m_lastBrowseTab = ui->tabWidget->currentIndex();

    UIUtils::clearLayout(ui->additionalWidgets);
    ui->historySearchWidget->clear();
    ui->historyWidget->clear();
    ui->homeWidget->clear();
    ui->searchWidget->clear();
    ui->subscriptionsWidget->clear();
    ui->trendingWidget->clear();

    switch (ui->tabWidget->currentIndex())
    {
    case TAB_HOME:
        ui->homeWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseHome(m_activePlugin, ui->homeWidget);
        break;
    case TAB_TRENDING:
        ui->trendingWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseTrending(m_activePlugin, ui->trendingWidget);
        break;
    case TAB_SUBSCRIPTIONS:
        ui->subscriptionsWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseSubscriptions(m_activePlugin, ui->subscriptionsWidget);
        break;
    case TAB_HISTORY:
        QLineEdit* historySearch = new QLineEdit(this);
        historySearch->setPlaceholderText("Search watch history");
        ui->additionalWidgets->addWidget(historySearch);
        connect(historySearch, &QLineEdit::returnPressed, this, &MainWindow::searchWatchHistory);

        BrowseHelper::instance()->browseHistory(m_activePlugin, ui->historyWidget);
        break;
    }
}

QStackedWidget* MainWindow::centralWidget()
{
    return ui->centralwidget;
}

void MainWindow::changeEvent(QEvent* event)
{
    // for some reason, the tab widget has to be completely manually repolished to update properly
    if (event->type() == QEvent::PaletteChange)
    {
        UIUtils::repolish(ui->tabWidget);

        const QList<QWidget*> children = ui->tabWidget->findChildren<QWidget*>();
        for (QWidget* widget : children)
            UIUtils::repolish(widget);
    }

    QWidget::changeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    if ((ctrlPressed && event->key() == Qt::Key_F) || (m_findbar->isVisible() && event->key() == Qt::Key_Escape))
        m_findbar->setReveal(m_findbar->isHidden());

    QMainWindow::keyPressEvent(event);
}

void MainWindow::pluginUpdateAvailable(const QString&, const ReleaseData&)
{
    m_topbar->updateNotificationCount(m_topbar->notificationBell->countText().toInt() + 1);
    // TODO: it would be nice to have this display a system notification as well.
    // that's why some stuff is being sent through the signal. not now though.
}

void MainWindow::reloadCurrentTab()
{
    if (ui->centralwidget->currentIndex() != 0 || !ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
        return;

    if (QWidget* widget = ui->tabWidget->currentWidget())
    {
        ContinuableListWidget* list = widget->findChild<ContinuableListWidget*>();
        if (!list || list->isPopulating())
            return;
    }

    if (ui->tabWidget->currentIndex() <= TAB_HISTORY)
    {
        browse();
    }
    else if (ui->tabWidget->currentIndex() == TAB_SEARCH)
    {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(
            m_activePlugin, ui->searchWidget, ui->additionalWidgets, m_lastSearchQuery);
    }
    else if (ui->tabWidget->currentIndex() == TAB_HISTORY_SEARCH)
    {
        searchWatchHistory();
    }
}

void MainWindow::reportJsException(const QString& str)
{
    if (ui->centralwidget->currentIndex() == 0)
    {
        if (QWidget* current = ui->tabWidget->currentWidget())
        {
            if (ContinuableListWidget* list = current->findChild<ContinuableListWidget*>())
            {
                list->clear();
                list->addItem(str);
                return;
            }
        }
    }

    QMessageBox::critical(this, "JS Error", str, QMessageBox::Ok);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    m_notificationMenu->setFixedSize(width() >= 800 ? 600 : 600 - (800 - width()), height() / 2);
    m_topbar->resize(width(), 35);
    m_topbar->scaleAppropriately();
    m_notificationMenu->move(m_topbar->notificationBell->x() - m_notificationMenu->width() + 20, 34);

    if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>())
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);

    QMainWindow::resizeEvent(event);
}

void MainWindow::returnFromSearch()
{
    UIUtils::clearLayout(ui->additionalWidgets);

    {
        QSignalBlocker blocker(ui->tabWidget);
        disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
        ui->tabWidget->setTabVisible(TAB_SEARCH, false);
        toggleTabsForActivePlugin();
    }

    ui->tabWidget->setCurrentIndex(m_lastBrowseTab);
    ui->searchWidget->clear();
}

void MainWindow::returnFromWatchHistorySearch()
{
    {
        QSignalBlocker blocker(ui->tabWidget);
        disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
        ui->tabWidget->setTabVisible(TAB_HISTORY_SEARCH, false);
        toggleTabsForActivePlugin();
    }

    ui->tabWidget->setCurrentIndex(TAB_HISTORY);
    ui->historySearchWidget->clear();
}

void MainWindow::search(const QString& query, SearchBox::SearchType searchType)
{
    if (query.isEmpty())
        return;

    if (searchType == SearchBox::SearchType::ByLink)
        qtTubeApp->handleUrlOrID(query);
    else
        searchByQuery(query);
}

void MainWindow::searchByQuery(const QString& query)
{
    m_topbar->setAlwaysShow(true);
    UIUtils::clearLayout(ui->additionalWidgets);
    ui->historySearchWidget->clear();
    ViewController::unloadCurrent();

    if (ui->tabWidget->currentIndex() == TAB_SEARCH)
    {
        ui->searchWidget->clear();
    }
    else
    {
        QSignalBlocker blocker(ui->tabWidget);
        connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
        ui->tabWidget->setTabVisible(TAB_SEARCH, true);
        UIUtils::setTabsVisible(ui->tabWidget, false,
            {TAB_HOME, TAB_TRENDING, TAB_SUBSCRIPTIONS, TAB_HISTORY, TAB_HISTORY_SEARCH});
        ui->tabWidget->setCurrentIndex(TAB_SEARCH);
    }

    m_lastSearchQuery = query;
    BrowseHelper::instance()->search(
        m_activePlugin, ui->searchWidget, ui->additionalWidgets, m_lastSearchQuery);
}

void MainWindow::searchWatchHistory()
{
    if (ui->tabWidget->currentIndex() == TAB_HISTORY_SEARCH)
    {
        ui->historySearchWidget->clear();
    }
    else
    {
        QSignalBlocker blocker(ui->tabWidget);
        connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
        ui->tabWidget->setTabVisible(TAB_HISTORY_SEARCH, true);
        UIUtils::setTabsVisible(ui->tabWidget, false,
            {TAB_HOME, TAB_TRENDING, TAB_SUBSCRIPTIONS, TAB_HISTORY, TAB_SEARCH});
        ui->tabWidget->setCurrentIndex(TAB_HISTORY_SEARCH);
    }

    m_lastSearchQuery = qobject_cast<QLineEdit*>(ui->additionalWidgets->itemAt(0)->widget())->text();
    BrowseHelper::instance()->browseHistory(m_activePlugin, ui->historySearchWidget, m_lastSearchQuery);
}

void MainWindow::toggleAccountMenu()
{
    assert(m_activePlugin != nullptr);

    if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>())
    {
        m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
        accountController->deleteLater();
        return;
    }

    m_topbar->setAlwaysShow(true);

    AccountControllerWidget* accountController = new AccountControllerWidget(m_activePlugin, this);
    accountController->show();
    accountController->raise();
    accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    connect(accountController, &AccountControllerWidget::resized, this, [this, accountController] {
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    });
}

void MainWindow::toggleNotificationMenu()
{
    if (m_notificationMenu->isVisible())
    {
        m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
        m_notificationMenu->clear();
        m_notificationMenu->hide();
        return;
    }

    m_topbar->setAlwaysShow(true);
    m_notificationMenu->show();

    for (const auto& [name, data] : qtTubeApp->plugins().updatablePlugins())
    {
        BrowseNotificationRenderer* notif = new BrowseNotificationRenderer;
        notif->setData(name, data);
        UIUtils::addWidgetToList(m_notificationMenu, notif);
    }

    if (m_activePlugin && m_activePlugin->providers.notifs && m_activePlugin->authenticated())
        BrowseHelper::instance()->browseNotificationMenu(m_activePlugin, m_notificationMenu);
}

void MainWindow::toggleTabsForActivePlugin()
{
    QSignalBlocker blocker(ui->tabWidget);
    m_topbar->searchBox->setEnabled(m_activePlugin && m_activePlugin->providers.search != nullptr);
    ui->tabWidget->setTabVisible(
        TAB_HOME, m_activePlugin && m_activePlugin->providers.home != nullptr);
    ui->tabWidget->setTabVisible(
        TAB_TRENDING, m_activePlugin && m_activePlugin->providers.trending != nullptr);
    ui->tabWidget->setTabVisible(
        TAB_SUBSCRIPTIONS, m_activePlugin && m_activePlugin->providers.subFeed != nullptr);
    ui->tabWidget->setTabVisible(
        TAB_HISTORY, m_activePlugin && m_activePlugin->providers.history != nullptr);

    // this is required if we're not on the "main page" of the program otherwise the layout breaks,
    // but i'm doing it even if we are on it, just in case.
    ui->tabWidget->tabBar()->adjustSize();
}

TopBar* MainWindow::topbar()
{
    return m_topbar;
}
