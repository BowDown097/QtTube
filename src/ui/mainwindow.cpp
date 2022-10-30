#include "../browsehelper.h"
#include "../settingsstore.h"
#include "innertube.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>
#include <QScrollBar>

namespace { MainWindow* mWInst; }
MainWindow* MainWindow::instance() { return mWInst; }

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    mWInst = this;
    setMouseTracking(true);
    setWindowIcon(QIcon(":/qttube.svg"));
    ui->setupUi(this);

    topbar = new TopBar(this);
    connect(topbar, &TopBar::signedIn, this, [this] { if (ui->centralwidget->currentIndex() == 0) browse(); });
    connect(topbar->searchBox, &QLineEdit::returnPressed, this, &MainWindow::search);

    ui->tabWidget->setTabEnabled(4, false);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::browse);

    connect(ui->historyWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseHistory>(value, ui->historyWidget); });
    connect(ui->homeWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseHome>(value, ui->homeWidget); });
    connect(ui->searchWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::Search>(value, ui->searchWidget, topbar->searchBox->text()); });
    connect(ui->subscriptionsWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseSubscriptions>(value, ui->subscriptionsWidget); });

    ui->historyWidget->verticalScrollBar()->setSingleStep(25);
    ui->homeWidget->verticalScrollBar()->setSingleStep(25);
    ui->searchWidget->verticalScrollBar()->setSingleStep(25);
    ui->subscriptionsWidget->verticalScrollBar()->setSingleStep(25);

    watchView = WatchView::instance();
    ui->centralwidget->addWidget(watchView);

    SettingsStore::instance().initializeFromSettingsFile();
    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220826.01.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    tryRestoreData();

    watchView->initialize(ui->centralwidget);
    if (SettingsStore::instance().frontPageTab != SettingsStore::FrontPageTab::None)
    {
        ui->tabWidget->setCurrentIndex(SettingsStore::instance().frontPageTab);
        browse();
    }
    else
    {
        ui->tabWidget->setCurrentIndex(4);
        setWindowTitle("QtTube");
    }
}

void MainWindow::browse()
{
    if (doNotBrowse)
        return;

    ui->historyWidget->clear();
    ui->homeWidget->clear();
    ui->searchWidget->clear();
    ui->subscriptionsWidget->clear();
    ui->trendingWidget->clear();

    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        setWindowTitle("Home - QtTube");
        BrowseHelper::instance().browseHome(ui->homeWidget);
        break;
    case 1:
        setWindowTitle("Trending - QtTube");
        BrowseHelper::instance().browseTrending(ui->trendingWidget);
        break;
    case 2:
        setWindowTitle("Subscriptions - QtTube");
        BrowseHelper::instance().browseSubscriptions(ui->subscriptionsWidget);
        break;
    case 3:
        setWindowTitle("History - QtTube");
        BrowseHelper::instance().browseHistory(ui->historyWidget);
        break;
    }
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    topbar->resize(width(), 35);
    topbar->searchBox->resize(440 + width() - 800, 35);
    topbar->settingsButton->move(topbar->searchBox->width() + topbar->searchBox->x() + 8, 0);
    topbar->signInButton->move(topbar->settingsButton->width() + topbar->settingsButton->x() + 8, 0);
}

void MainWindow::returnFromSearch()
{
    doNotBrowse = true;
    disconnect(topbar->logo, &ClickableLabel::clicked, this, &MainWindow::returnFromSearch);
    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(0, true);
    ui->tabWidget->setTabEnabled(1, true);
    ui->tabWidget->setTabEnabled(2, true);
    ui->tabWidget->setTabEnabled(3, true);
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::search()
{
    if (ui->centralwidget->currentIndex() == 1)
        WatchView::instance()->goBack();

    if (ui->tabWidget->currentIndex() == 4)
    {
        ui->searchWidget->clear();
        BrowseHelper::instance().search(ui->searchWidget, topbar->searchBox->text());
        return;
    }

    doNotBrowse = true;
    connect(topbar->logo, &ClickableLabel::clicked, this, &MainWindow::returnFromSearch);
    ui->tabWidget->setTabEnabled(4, true);
    ui->tabWidget->setTabEnabled(0, false);
    ui->tabWidget->setTabEnabled(1, false);
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(4);
    BrowseHelper::instance().search(ui->searchWidget, topbar->searchBox->text());
}

void MainWindow::tryRestoreData()
{
    QFile store(SettingsStore::configPath.filePath("store.json"));
    if (!store.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream storeOut(&store);
    QString storeData = storeOut.readAll();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(storeData.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return;

    InnerTube::instance().authenticateFromJson(doc.object());
    topbar->signInButton->setText("Sign out");
}

MainWindow::~MainWindow()
{
    delete ui;
}

