#include "include/innertube/requests.hpp"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InnertubeClient client("WEB", "2.20220719", "DESKTOP", "USER_INTERFACE_THEME_DARK");
    InnertubeContext context(client);

    QString data = InnertubeRequests::browse(context);
    QFile file("out.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << data;
    file.close();
}

MainWindow::~MainWindow()
{
    delete ui;
}

