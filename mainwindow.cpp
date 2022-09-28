#include "browsehelper.hpp"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/settingsform.h"

namespace { MainWindow* mWInst; }
MainWindow* MainWindow::instance()  { return mWInst; }

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    mWInst = this;
    ui->setupUi(this);

    ui->tabWidget->setTabEnabled(4, false);
    ui->searchReturnButton->setVisible(false);

    connect(ui->searchBox, &QLineEdit::returnPressed, this, &MainWindow::search);
    connect(ui->searchReturnButton, &QPushButton::clicked, this, &MainWindow::returnFromSearch);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(ui->signInButton, &QPushButton::clicked, this, &MainWindow::signinClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::browse);

    connect(ui->historyWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseHistory>(value, ui->historyWidget); });
    connect(ui->homeWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseHome>(value, ui->homeWidget); });
    connect(ui->searchWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::Search>(value, ui->searchWidget, ui->searchBox->text()); });
    connect(ui->subscriptionsWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseSubscriptions>(value, ui->subscriptionsWidget); });

    QAction* sneed = new QAction;
    sneed->setShortcut(QKeySequence(Qt::Key_S, Qt::Key_E, Qt::Key_E, Qt::Key_D));
    connect(sneed, &QAction::triggered, this,
            [] { WatchView::instance()->loadVideo(InnerTube::instance().get<InnertubeEndpoints::Player>("T3ElIvNBYj0")); });
    addAction(sneed);

    ui->historyWidget->verticalScrollBar()->setSingleStep(25);
    ui->homeWidget->verticalScrollBar()->setSingleStep(25);
    ui->searchWidget->verticalScrollBar()->setSingleStep(25);
    ui->subscriptionsWidget->verticalScrollBar()->setSingleStep(25);

    watchView = WatchView::instance();
    ui->centralwidget->addWidget(watchView);

    SettingsStore::instance().initializeFromSettingsFile();
    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220826.01.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    tryRestoreData();

    BrowseHelper::instance().browseHome(ui->homeWidget);
    watchView->initialize(ui->centralwidget);
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
        setWindowTitle("Home - youtube-qt");
        BrowseHelper::instance().browseHome(ui->homeWidget);
        break;
    case 1:
        setWindowTitle("Trending - youtube-qt");
        break;
    case 2:
        setWindowTitle("Subscriptions - youtube-qt");
        BrowseHelper::instance().browseSubscriptions(ui->subscriptionsWidget);
        break;
    case 3:
        setWindowTitle("History - youtube-qt");
        BrowseHelper::instance().browseHistory(ui->historyWidget);
        break;
    }
}

void MainWindow::returnFromSearch()
{
    doNotBrowse = true;
    ui->searchReturnButton->setVisible(false);
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
    if (ui->tabWidget->currentIndex() == 4)
    {
        ui->searchWidget->clear();
        BrowseHelper::instance().search(ui->searchWidget, ui->searchBox->text());
        return;
    }

    doNotBrowse = true;
    ui->searchReturnButton->setVisible(true);
    ui->tabWidget->setTabEnabled(4, true);
    ui->tabWidget->setTabEnabled(0, false);
    ui->tabWidget->setTabEnabled(1, false);
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(4);
    BrowseHelper::instance().search(ui->searchWidget, ui->searchBox->text());
}

void MainWindow::showSettings()
{
    SettingsForm* settings = new SettingsForm;
    settings->show();
}

void MainWindow::signinClicked()
{
    if (InnerTube::instance().hasAuthenticated())
        return;

    InnerTube::instance().authenticate();
    if (SettingsStore::instance().itcCache)
    {
        QFile store("store.json");
        if (store.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream storeIn(&store);
            storeIn << QJsonDocument(InnerTube::instance().authStore()->toJson()).toJson(QJsonDocument::Compact);
        }
    }

    ui->signInButton->setText("Sign out");
    browse();
}

void MainWindow::tryRestoreData()
{
    QFile store("store.json");
    if (!store.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream storeOut(&store);
    QString storeData = storeOut.readAll();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(storeData.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return;

    InnerTube::instance().authStore()->populateFromJson(doc.object(), *InnerTube::instance().context());
    ui->signInButton->setText("Sign out");
}

MainWindow::~MainWindow()
{
    delete ui;
}

