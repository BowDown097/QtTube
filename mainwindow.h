#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "innertube.hpp"
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
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
