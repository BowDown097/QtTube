#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "innertube/auth.hpp"
#include <QMainWindow>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void signinClicked();
private:
    InnertubeAuthStore* authStore = new InnertubeAuthStore;
    InnertubeContext context = InnertubeContext(InnertubeClient("WEB", "2.20220720.00.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
