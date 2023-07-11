#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "credentialsstore.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/browsehelper.h"
#include "ui/views/channelview.h"
#include "ui/views/viewcontroller.h"
#include "ui/views/watchview.h"
#include "ui/widgets/accountmenu/accountcontrollerwidget.h"
#include "ui/uiutilities.h"
#include <QComboBox>
#include <QScrollBar>

MainWindow::MainWindow(const QCommandLineParser& parser, QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_centralWidget = ui->centralwidget;
    m_size = geometry().size();
    m_topbar = new TopBar(this);
#ifdef Q_OS_WIN
    m_winId = winId();
#endif

    notificationMenu = new QListWidget(this);
    notificationMenu->setVisible(false);

    findbar = new FindBar(this);
    connect(ui->centralwidget, &QStackedWidget::currentChanged, this, [this] { if (findbar->isVisible()) { findbar->setReveal(false); } });

    connect(m_topbar, &TopBar::signInStatusChanged, this, [this] { if (ui->centralwidget->currentIndex() == 0) browse(); });
    connect(m_topbar->avatarButton, &TubeLabel::clicked, this, &MainWindow::showAccountMenu);
    connect(m_topbar->notificationBell, &TopBarBell::clicked, this, &MainWindow::showNotifications);
    connect(m_topbar->searchBox, &QLineEdit::returnPressed, this, &MainWindow::search);

    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setCurrentIndex(5); // just some blank tab so you can pick one
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::browse);

    connect(notificationMenu->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        BrowseHelper::instance()->tryContinuation<InnertubeEndpoints::GetNotificationMenu>(value, notificationMenu, "NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX", 5);
    });
    connect(ui->historyWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        BrowseHelper::instance()->tryContinuation<InnertubeEndpoints::BrowseHistory>(value, ui->historyWidget, lastSearchQuery);
    });
    connect(ui->homeWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        BrowseHelper::instance()->tryContinuation<InnertubeEndpoints::BrowseHome>(value, ui->homeWidget);
    });
    connect(ui->searchWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        BrowseHelper::instance()->tryContinuation<InnertubeEndpoints::Search>(value, ui->searchWidget, lastSearchQuery);
    });
    connect(ui->subscriptionsWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        BrowseHelper::instance()->tryContinuation<InnertubeEndpoints::BrowseSubscriptions>(value, ui->subscriptionsWidget);
    });

    ui->historySearchWidget->verticalScrollBar()->setSingleStep(25);
    ui->historyWidget->verticalScrollBar()->setSingleStep(25);
    ui->homeWidget->verticalScrollBar()->setSingleStep(25);
    ui->searchWidget->verticalScrollBar()->setSingleStep(25);
    ui->subscriptionsWidget->verticalScrollBar()->setSingleStep(25);
    ui->trendingWidget->verticalScrollBar()->setSingleStep(25);

    QAction* reloadShortcut = new QAction(this);
    reloadShortcut->setAutoRepeat(false);
    reloadShortcut->setShortcuts(QList<QKeySequence>() << Qt::Key_F5 << QKeySequence(Qt::ControlModifier | Qt::Key_R));
    connect(reloadShortcut, &QAction::triggered, this, &MainWindow::reloadCurrentTab);
    addAction(reloadShortcut);

    CredentialsStore::instance()->initializeFromStoreFile();
    SettingsStore::instance()->initializeFromSettingsFile();
    UIUtilities::defaultStyle = qApp->style()->objectName();
    UIUtilities::setAppStyle(SettingsStore::instance()->appStyle, SettingsStore::instance()->darkTheme);

    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220826.01.00", "DESKTOP"));
    tryRestoreData();

    if (parser.isSet("channel"))
        ViewController::loadChannel(parser.value("channel"));
    else if (parser.isSet("video"))
        ViewController::loadVideo(parser.value("video"));
}

void MainWindow::browse()
{
    if (doNotBrowse)
        return;

    UIUtilities::clearLayout(ui->additionalWidgets);
    ui->historySearchWidget->clear();
    ui->historyWidget->clear();
    ui->homeWidget->clear();
    ui->searchWidget->clear();
    ui->subscriptionsWidget->clear();
    ui->trendingWidget->clear();

    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        BrowseHelper::instance()->browseHome(ui->homeWidget);
        break;
    case 1:
        BrowseHelper::instance()->browseTrending(ui->trendingWidget);
        break;
    case 2:
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
    if (ui->centralwidget->currentIndex() == 0)
    {
        bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
        if ((ctrlPressed && event->key() == Qt::Key_F) || event->key() == Qt::Key_Escape)
            findbar->setReveal(!findbar->isVisible());
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::performFilteredSearch()
{
    ui->searchWidget->clear();
    int dateIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(1)->widget())->currentIndex();
    int typeIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(2)->widget())->currentIndex();
    int durIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(3)->widget())->currentIndex();
    int featIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(4)->widget())->currentIndex();
    int sortIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(5)->widget())->currentIndex();
    BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateIndex, typeIndex, durIndex, featIndex, sortIndex);
}

void MainWindow::reloadCurrentTab()
{
    if (ui->centralwidget->currentIndex() != 0 || !ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
        return;

    if (ui->tabWidget->currentIndex() <= 3)
        browse();
    else if (ui->tabWidget->currentIndex() == 4)
        performFilteredSearch();
    else if (ui->tabWidget->currentIndex() == 5)
        searchWatchHistory();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    m_size = event->size();
    notificationMenu->setFixedSize(width() >= 800 ? 600 : 600 - (800 - width()), height() / 2);
    m_topbar->resize(width(), 35);
    m_topbar->scaleAppropriately();
    notificationMenu->move(m_topbar->notificationBell->x() - notificationMenu->width() + 20, 34);

    if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>("accountController"))
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
}

void MainWindow::returnFromSearch()
{
    UIUtilities::clearLayout(ui->additionalWidgets);
    doNotBrowse = true;
    disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
    ui->tabWidget->setTabEnabled(4, false);
    UIUtilities::setTabsEnabled(ui->tabWidget, true, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(0);
    ui->searchWidget->clear();
}

void MainWindow::returnFromWatchHistorySearch()
{
    doNotBrowse = true;
    disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
    ui->tabWidget->setTabEnabled(5, false);
    UIUtilities::setTabsEnabled(ui->tabWidget, true, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(3);
    ui->historySearchWidget->clear();
}

void MainWindow::search()
{
    m_topbar->alwaysShow = true;
    UIUtilities::clearLayout(ui->additionalWidgets);
    ui->historySearchWidget->clear();

    if (ChannelView* channelView = qobject_cast<ChannelView*>(ui->centralwidget->currentWidget()))
        channelView->deleteLater();
    else if (WatchView* watchView = qobject_cast<WatchView*>(ui->centralwidget->currentWidget()))
        watchView->deleteLater();

    TubeLabel* filtersLabel = new TubeLabel("Filters:", this);
    ui->additionalWidgets->addWidget(filtersLabel);

    QComboBox* dateCmb = new QComboBox(this);
    dateCmb->setPlaceholderText("Upload date");
    dateCmb->addItems({"Last hour", "Today", "This week", "This month", "This year"});
    ui->additionalWidgets->addWidget(dateCmb);

    QComboBox* typeCmb = new QComboBox(this);
    typeCmb->setPlaceholderText("Type");
    typeCmb->addItems({"Video", "Channel", "Playlist", "Movie"});
    ui->additionalWidgets->addWidget(typeCmb);

    QComboBox* durCmb = new QComboBox(this);
    durCmb->setPlaceholderText("Duration");
    durCmb->addItems({"Under 4 minutes", "Over 20 minutes", "4-20 minutes"});
    ui->additionalWidgets->addWidget(durCmb);

    QComboBox* featCmb = new QComboBox(this);
    featCmb->setPlaceholderText("Features");
    featCmb->addItems({"Live", "4K", "HD", "Subtitles/CC", "Creative Commons", "360°", "VR180", "3D", "HDR", "Location", "Purchased"});
    ui->additionalWidgets->addWidget(featCmb);

    QComboBox* sortCmb = new QComboBox(this);
    sortCmb->setPlaceholderText("Sort by");
    sortCmb->addItems({"Relevance", "Rating", "Upload date", "View count"});
    ui->additionalWidgets->addWidget(sortCmb);

    if (ui->tabWidget->currentIndex() == 4)
    {
        ui->searchWidget->clear();
    }
    else
    {
        doNotBrowse = true;
        connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
        ui->tabWidget->setTabEnabled(4, true);
        UIUtilities::setTabsEnabled(ui->tabWidget, false, {0, 1, 2, 3, 5});
        doNotBrowse = false;
        ui->tabWidget->setCurrentIndex(4);
    }

    lastSearchQuery = m_topbar->searchBox->text();
    BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery);

    connect(dateCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(typeCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(durCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(featCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(sortCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
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
    UIUtilities::setTabsEnabled(ui->tabWidget, false, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(5);

    lastSearchQuery = qobject_cast<QLineEdit*>(ui->additionalWidgets->itemAt(0)->widget())->text();
    BrowseHelper::instance()->browseHistory(ui->historySearchWidget, lastSearchQuery);
}

void MainWindow::showAccountMenu()
{
    if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>("accountController"))
    {
        if (ui->centralwidget->currentIndex() != 0)
            m_topbar->alwaysShow = false;
        accountController->deleteLater();
        return;
    }

    m_topbar->alwaysShow = true;

    AccountControllerWidget* accountController = new AccountControllerWidget(this);
    accountController->setObjectName("accountController");
    accountController->show();
    accountController->raise();
    accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    connect(accountController, &AccountControllerWidget::resized, this, [this, accountController] {
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    });

    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::AccountMenu>();
    connect(reply, qOverload<const InnertubeEndpoints::AccountMenu&>(&InnertubeReply::finished), accountController->accountMenu,
            &AccountMenuWidget::initialize);
}

void MainWindow::showNotifications()
{
    if (notificationMenu->isVisible())
    {
        if (ui->centralwidget->currentIndex() != 0)
            m_topbar->alwaysShow = false;
        notificationMenu->clear();
        notificationMenu->setVisible(false);
        return;
    }

    m_topbar->alwaysShow = true;
    notificationMenu->setVisible(true);
    BrowseHelper::instance()->browseNotificationMenu(notificationMenu);
}

void MainWindow::tryRestoreData()
{
    CredentialsStore::instance()->populateAuthStore(CredentialsStore::instance()->getActiveLoginIndex());
    if (InnerTube::instance().hasAuthenticated())
    {
        m_topbar->avatarButton->setVisible(true);
        m_topbar->signInButton->setVisible(false);
        m_topbar->setUpAvatarButton();
        m_topbar->setUpNotifications();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
