#include "browsehelper.hpp"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/settingsform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(ui->signInButton, &QPushButton::clicked, this, &MainWindow::signinClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);

    connect(ui->homeWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseHome>(value, ui->homeWidget); });
    connect(ui->subscriptionsWidget->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) { BrowseHelper::instance().tryContinuation<InnertubeEndpoints::BrowseSubscriptions>(value, ui->subscriptionsWidget); });
    SettingsStore::instance().initializeFromSettingsFile();
    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220826.01.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    BrowseHelper::instance().browseHome(ui->homeWidget);
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
    ui->signInButton->setText("Sign out");

    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        ui->homeWidget->clear();
        BrowseHelper::instance().browseHome(ui->homeWidget);
        break;
    case 1:
        ui->trendingWidget->clear();
        break;
    case 2:
        ui->subscriptionsWidget->clear();
        BrowseHelper::instance().browseSubscriptions(ui->subscriptionsWidget);
        break;
    case 3:
        ui->historyWidget->clear();
        break;
    }
}

void MainWindow::tabChanged(int index)
{
    switch (index)
    {
    case 0:
        ui->homeWidget->clear();
        BrowseHelper::instance().browseHome(ui->homeWidget);
        break;
    case 1:
        ui->trendingWidget->clear();
        break;
    case 2:
        ui->subscriptionsWidget->clear();
        BrowseHelper::instance().browseSubscriptions(ui->subscriptionsWidget);
        break;
    case 3:
        ui->historyWidget->clear();
        break;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

