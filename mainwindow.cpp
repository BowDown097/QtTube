#include "innertube/requests.hpp"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // QString data = InnertubeRequests::browse(context);
    connect(ui->signInButton, &QPushButton::clicked, this, &MainWindow::signinClicked);
}

void MainWindow::signinClicked()
{
    if (authStore->populated)
        return;

    authStore->authenticate(context);
    ui->signInButton->setText("Sign out");
    QString data = InnertubeRequests::browse(context, *authStore);
    qDebug() << data.left(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}

