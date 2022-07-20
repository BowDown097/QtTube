#include "include/innertube/requests.hpp"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InnertubeClient client("Firefox", "103.0", "UNKNOWN_FORM_FACTOR", "WEB", "2.20220719", InnertubeConfigInfo(""), "", "", "US", "en",
                           "https://www.youtube.com/", "X11", "", "DESKTOP", "", 2, 2, "", "", "USER_INTERFACE_THEME_DARK", "");
    InnertubeContext context(InnertubeClickTracking(""), client, InnertubeRequestConfig(true), InnertubeUserConfig(false));

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

