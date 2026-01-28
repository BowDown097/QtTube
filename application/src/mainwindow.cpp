#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qttubeapplication.h"
#include "ui/browsehelper.h"
#include "ui/forms/plugins/pluginbrowserview.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/accountmenu/accountcontrollerwidget.h"
#include "utils/uiutils.h"
#include <QAction>
#include <QLineEdit>
#include <QMessageBox>

MainWindow::~MainWindow() { delete ui; }

MainWindow::MainWindow(const QCommandLineParser& parser, QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QTTUBE_APP_NAME);

    m_centralWidget = ui->centralwidget;
    m_size = geometry().size();
    m_topbar = new TopBar(this);

    notificationMenu = new ContinuableListWidget(this);
    notificationMenu->hide();
    notificationMenu->setContinuationThreshold(5);

    findbar = new FindBar(this);
    connect(ui->centralwidget, &QStackedWidget::currentChanged, this, [this] {
        if (findbar->isVisible())
            findbar->setReveal(false);
    });

    connect(m_topbar, &TopBar::signInStatusChanged, this, [this] { if (ui->centralwidget->currentIndex() == 0) browse(); });
    connect(m_topbar->avatarButton, &TubeLabel::clicked, this, &MainWindow::showAccountMenu);
    connect(m_topbar->notificationBell, &TopBarBell::clicked, this, &MainWindow::showNotifications);
    connect(m_topbar->searchBox, &SearchBox::searchRequested, this, &MainWindow::search);

    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setCurrentIndex(5); // just some blank tab so you can pick one
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::browse);

    connect(notificationMenu, &ContinuableListWidget::continuationReady, this, [this] {
        if (notificationMenu->continuationData.has_value())
            BrowseHelper::instance()->browseNotificationMenu(notificationMenu);
    });
    connect(ui->historyWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->historyWidget->continuationData.has_value())
            BrowseHelper::instance()->browseHistory(ui->historyWidget, lastSearchQuery);
    });
    connect(ui->homeWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->homeWidget->continuationData.has_value())
            BrowseHelper::instance()->browseHome(ui->homeWidget);
    });
    connect(ui->searchWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->searchWidget->continuationData.has_value())
            BrowseHelper::instance()->search(ui->searchWidget, nullptr, lastSearchQuery);
    });
    connect(ui->subscriptionsWidget, &ContinuableListWidget::continuationReady, this, [this] {
        if (ui->subscriptionsWidget->continuationData.has_value())
            BrowseHelper::instance()->browseSubscriptions(ui->subscriptionsWidget);
    });

    QAction* reloadShortcut = new QAction(this);
    reloadShortcut->setAutoRepeat(false);
    reloadShortcut->setShortcuts(QList<QKeySequence>() << Qt::Key_F5 << QKeySequence(Qt::ControlModifier | Qt::Key_R));
    connect(reloadShortcut, &QAction::triggered, this, &MainWindow::reloadCurrentTab);
    addAction(reloadShortcut);

    qtTubeApp->doInitialSetup();
    connect(qtTubeApp, &QtTubeApplication::activePluginChanged, this, &MainWindow::activePluginChanged);

#ifdef Q_OS_LINUX
    if (qtTubeApp->settings().playerSettings.vaapi)
    {
        qputenv("LIBVA_DRI3_DISABLE", "1"); // fixes issue on some older GPUs
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", qgetenv("QTWEBENGINE_CHROMIUM_FLAGS") + " --enable-features=VaapiVideoDecoder --enable-features=VaapiIgnoreDriverChecks --disable-features=UseChromeOSDirectVideoDecoder");
    }
#endif

    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        // just call activePluginChanged() to do setup for whatever plugin has been loaded
        activePluginChanged(plugin);

        if (parser.isSet("channel"))
            ViewController::loadChannel(parser.value("channel"), plugin);
        else if (parser.isSet("video"))
            ViewController::loadVideo(parser.value("video"), plugin);
    }
    else if (!qtTubeApp->plugins().hasLoadablePlugins())
    {
        // wrapped in this manner to avoid blocking displaying of the main window
        QMetaObject::invokeMethod(this, [this] {
            if (QMessageBox::question(this, "Browse Plugins?", "You have no plugins installed. Would you like to open the plugin browser?") == QMessageBox::Yes)
            {
                PluginBrowserView::spawn();
            }
        }, Qt::QueuedConnection);
    }
}

void MainWindow::activePluginChanged(PluginData* activePlugin)
{
    if (QtTubePlugin::AuthStoreBase* authStore = activePlugin->auth)
    {
        if (const QtTubePlugin::AuthUser* authUser = authStore->activeBaseLogin())
        {
            m_topbar->avatarButton->setImage(authUser->avatar, TubeLabel::Cached | TubeLabel::Rounded);
            m_topbar->postSignInSetup();
        }
        else
        {
            m_topbar->updateUIForSignInState(false);
        }

        connect(authStore, &QtTubePlugin::AuthStoreBase::authenticateSuccess, m_topbar, &TopBar::postSignInSetup);
        connect(authStore, &QtTubePlugin::AuthStoreBase::updateFail, this, [] {
            QMessageBox::critical(nullptr, "Invalid Login", "Your session has expired or your credentials are invalid. You will be logged out. Try logging in again.");
            m_topbar->signOut();
        });
    }
    else
    {
        m_topbar->updateUIForSignInState(false);
    }
}

void MainWindow::browse()
{
    if (doNotBrowse)
        return;

    UIUtils::clearLayout(ui->additionalWidgets);
    ui->historySearchWidget->clear();
    ui->historyWidget->clear();
    ui->homeWidget->clear();
    ui->searchWidget->clear();
    ui->subscriptionsWidget->clear();
    ui->trendingWidget->clear();

    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        ui->homeWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseHome(ui->homeWidget);
        break;
    case 1:
        ui->trendingWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseTrending(ui->trendingWidget);
        break;
    case 2:
        ui->subscriptionsWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseSubscriptions(ui->subscriptionsWidget);
        break;
    case 3:
        QLineEdit* historySearch = new QLineEdit(this);
        historySearch->setPlaceholderText("Search watch history");
        ui->additionalWidgets->addWidget(historySearch);
        connect(historySearch, &QLineEdit::returnPressed, this, &MainWindow::searchWatchHistory);

        BrowseHelper::instance()->browseHistory(ui->historyWidget);
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    if ((ctrlPressed && event->key() == Qt::Key_F) || (findbar->isVisible() && event->key() == Qt::Key_Escape))
        findbar->setReveal(findbar->isHidden());

    QMainWindow::keyPressEvent(event);
}

void MainWindow::reloadCurrentTab()
{
    if (ui->centralwidget->currentIndex() != 0 || !ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
        return;

    if (QWidget* widget = ui->tabWidget->widget(ui->tabWidget->currentIndex()))
        if (ContinuableListWidget* list = widget->findChild<ContinuableListWidget*>(); list->isPopulating())
            return;

    if (ui->tabWidget->currentIndex() <= 3)
    {
        browse();
    }
    else if (ui->tabWidget->currentIndex() == 4)
    {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, ui->additionalWidgets, lastSearchQuery);
    }
    else
    {
        searchWatchHistory();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    m_size = event->size();
    notificationMenu->setFixedSize(width() >= 800 ? 600 : 600 - (800 - width()), height() / 2);
    m_topbar->resize(width(), 35);
    m_topbar->scaleAppropriately();
    notificationMenu->move(m_topbar->notificationBell->x() - notificationMenu->width() + 20, 34);

    if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>())
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);

    QMainWindow::resizeEvent(event);
}

void MainWindow::returnFromSearch()
{
    UIUtils::clearLayout(ui->additionalWidgets);
    doNotBrowse = true;
    disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
    ui->tabWidget->setTabEnabled(4, false);
    UIUtils::setTabsEnabled(ui->tabWidget, true, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(0);
    ui->searchWidget->clear();
}

void MainWindow::returnFromWatchHistorySearch()
{
    doNotBrowse = true;
    disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
    ui->tabWidget->setTabEnabled(5, false);
    UIUtils::setTabsEnabled(ui->tabWidget, true, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(3);
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

    if (ui->tabWidget->currentIndex() == 4)
    {
        ui->searchWidget->clear();
    }
    else
    {
        doNotBrowse = true;
        connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
        ui->tabWidget->setTabEnabled(4, true);
        UIUtils::setTabsEnabled(ui->tabWidget, false, {0, 1, 2, 3, 5});
        doNotBrowse = false;
        ui->tabWidget->setCurrentIndex(4);
    }

    lastSearchQuery = query;
    BrowseHelper::instance()->search(ui->searchWidget, ui->additionalWidgets, lastSearchQuery);
}

void MainWindow::searchWatchHistory()
{
    if (ui->tabWidget->currentIndex() == 5)
    {
        ui->historySearchWidget->clear();
        lastSearchQuery = qobject_cast<QLineEdit*>(ui->additionalWidgets->itemAt(0)->widget())->text();
        BrowseHelper::instance()->browseHistory(ui->historySearchWidget, lastSearchQuery);
        return;
    }

    doNotBrowse = true;
    connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
    ui->tabWidget->setTabEnabled(5, true);
    UIUtils::setTabsEnabled(ui->tabWidget, false, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(5);

    lastSearchQuery = qobject_cast<QLineEdit*>(ui->additionalWidgets->itemAt(0)->widget())->text();
    BrowseHelper::instance()->browseHistory(ui->historySearchWidget, lastSearchQuery);
}

void MainWindow::showAccountMenu()
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>())
        {
            m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
            accountController->deleteLater();
            return;
        }

        m_topbar->setAlwaysShow(true);

        AccountControllerWidget* accountController = new AccountControllerWidget(plugin, this);
        accountController->show();
        accountController->raise();
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
        connect(accountController, &AccountControllerWidget::resized, this, [this, accountController] {
            accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
        });
    }
}

void MainWindow::showNotifications()
{
    if (notificationMenu->isVisible())
    {
        m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
        notificationMenu->clear();
        notificationMenu->hide();
        return;
    }

    m_topbar->setAlwaysShow(true);
    notificationMenu->show();
    BrowseHelper::instance()->browseNotificationMenu(notificationMenu);
}
