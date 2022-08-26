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
    SettingsStore::instance().initializeFromSettingsFile();
    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220823.05.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    BrowseHelper::browseHome(ui->homeWidget);
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
    ui->homeWidget->clear();
    BrowseHelper::browseHome(ui->homeWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

