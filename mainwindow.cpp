#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220721.00.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    QString data = InnerTube::instance().get<InnertubeEndpoints::NextVideo>("sjMuL5XEZzs").data;
    qDebug() << data.left(10000);
    connect(ui->signInButton, &QPushButton::clicked, this, &MainWindow::signinClicked);
}

void MainWindow::signinClicked()
{
    if (InnerTube::instance().hasAuthenticated())
        return;

    InnerTube::instance().authenticate();
    ui->signInButton->setText("Sign out");
    QString data = InnerTube::instance().get<InnertubeEndpoints::Player>("sjMuL5XEZzs").data;
    qDebug() << data.left(10000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

