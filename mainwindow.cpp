#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220720.00.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    QString data = InnerTube::instance().get<InnertubeEndpoints::BrowseSubscriptions>().data;
    qDebug() << data.left(10000);
    connect(ui->signInButton, &QPushButton::clicked, this, &MainWindow::signinClicked);
}

void MainWindow::signinClicked()
{
    if (InnerTube::instance().authStore()->populated)
        return;

    InnerTube::instance().authStore()->authenticate(*InnerTube::instance().context());
    ui->signInButton->setText("Sign out");
    QString data = InnerTube::instance().get<InnertubeEndpoints::BrowseSubscriptions>().data;
    qDebug() << data.left(10000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

