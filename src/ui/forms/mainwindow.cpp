#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/browsehelper.h"
#include "ui/views/channelview.h"
#include "ui/views/watchview.h"
#include "ui/uiutilities.h"
#include <QComboBox>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_centralWidget = ui->centralwidget;

    notificationMenu = new QListWidget(this);
    notificationMenu->setVisible(false);

    findbar = new FindBar(this);
    connect(ui->centralwidget, &QStackedWidget::currentChanged, this, [this] { if (findbar->isVisible()) { findbar->setReveal(false); } });

    m_topbar = new TopBar(this);
    connect(m_topbar, &TopBar::notificationBellClicked, this, &MainWindow::showNotifications);
    connect(m_topbar, &TopBar::signInStatusChanged, this, [this] { if (ui->centralwidget->currentIndex() == 0) browse(); });
    connect(m_topbar->searchBox, &QLineEdit::returnPressed, this, &MainWindow::search);

    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
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

    SettingsStore::instance().initializeFromSettingsFile();
    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220826.01.00", "DESKTOP"));
    tryRestoreData();

    ui->tabWidget->setCurrentIndex(5); // just some blank tab so you can pick one
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
        QLineEdit* historySearch = new QLineEdit;
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

void MainWindow::resizeEvent(QResizeEvent* event)
{
    m_size = event->size();
    notificationMenu->setFixedSize(width() >= 800 ? 600 : 600 - (800 - width()), height() / 2);
    m_topbar->resize(width(), 35);
    m_topbar->scaleAppropriately();
    notificationMenu->move(m_topbar->notificationBell->x() - notificationMenu->width() + 20, 34);
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

    TubeLabel* filtersLabel = new TubeLabel("Filters:");
    ui->additionalWidgets->addWidget(filtersLabel);

    QComboBox* dateCmb = new QComboBox;
    dateCmb->setPlaceholderText("Upload date");
    dateCmb->addItems({"Last hour", "Today", "This week", "This month", "This year"});
    ui->additionalWidgets->addWidget(dateCmb);

    QComboBox* typeCmb = new QComboBox;
    typeCmb->setPlaceholderText("Type");
    typeCmb->addItems({"Video", "Channel", "Playlist", "Movie"});
    ui->additionalWidgets->addWidget(typeCmb);

    QComboBox* durCmb = new QComboBox;
    durCmb->setPlaceholderText("Duration");
    durCmb->addItems({"Under 4 minutes", "Over 20 minutes", "4-20 minutes"});
    ui->additionalWidgets->addWidget(durCmb);

    QComboBox* featCmb = new QComboBox;
    featCmb->setPlaceholderText("Features");
    featCmb->addItems({"Live", "4K", "HD", "Subtitles/CC", "Creative Commons", "360°", "VR180", "3D", "HDR", "Location", "Purchased"});
    ui->additionalWidgets->addWidget(featCmb);

    QComboBox* sortCmb = new QComboBox;
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(dateCmb, &QComboBox::currentIndexChanged, this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, index, typeCmb->currentIndex(), durCmb->currentIndex(),
                                        featCmb->currentIndex(), sortCmb->currentIndex());
    });
    connect(typeCmb, &QComboBox::currentIndexChanged, this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), index, durCmb->currentIndex(),
                                        featCmb->currentIndex(), sortCmb->currentIndex());
    });
    connect(durCmb, &QComboBox::currentIndexChanged, this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), typeCmb->currentIndex(), index,
                                        featCmb->currentIndex(), sortCmb->currentIndex());
    });
    connect(featCmb, &QComboBox::currentIndexChanged, this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), typeCmb->currentIndex(),
                                        durCmb->currentIndex(), index, sortCmb->currentIndex());
    });
    connect(sortCmb, &QComboBox::currentIndexChanged, this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), typeCmb->currentIndex(),
                                        durCmb->currentIndex(), featCmb->currentIndex(), index);
    });
#else
    connect(dateCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, index, typeCmb->currentIndex(), durCmb->currentIndex(),
                                        featCmb->currentIndex(), sortCmb->currentIndex());
    });
    connect(typeCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), index, durCmb->currentIndex(),
                                        featCmb->currentIndex(), sortCmb->currentIndex());
    });
    connect(durCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), typeCmb->currentIndex(), index,
                                        featCmb->currentIndex(), sortCmb->currentIndex());
    });
    connect(featCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), typeCmb->currentIndex(),
                                        durCmb->currentIndex(), index, sortCmb->currentIndex());
    });
    connect(sortCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this](int index) {
        ui->searchWidget->clear();
        BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateCmb->currentIndex(), typeCmb->currentIndex(),
                                        durCmb->currentIndex(), featCmb->currentIndex(), index);
    });
#endif
}

void MainWindow::searchWatchHistory()
{
    if (ui->tabWidget->currentIndex() == 5)
    {
        ui->historySearchWidget->clear();
        lastSearchQuery = qobject_cast<QLineEdit*>(sender())->text();
        BrowseHelper::instance()->browseHistory(ui->historySearchWidget, lastSearchQuery);
        return;
    }

    doNotBrowse = true;
    connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
    ui->tabWidget->setTabEnabled(5, true);
    UIUtilities::setTabsEnabled(ui->tabWidget, false, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(5);

    lastSearchQuery = qobject_cast<QLineEdit*>(sender())->text();
    BrowseHelper::instance()->browseHistory(ui->historySearchWidget, lastSearchQuery);
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
    QSettings store(SettingsStore::configPath.filePath("store.ini"), QSettings::IniFormat);
    InnerTube::instance().authenticateFromSettings(store);
    if (InnerTube::instance().hasAuthenticated())
    {
        m_topbar->setUpNotifications();
        m_topbar->signInButton->setText("Sign out");
        disconnect(m_topbar->signInButton, &QPushButton::clicked, m_topbar, &TopBar::trySignIn);
        connect(m_topbar->signInButton, &QPushButton::clicked, m_topbar, &TopBar::signOut);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
