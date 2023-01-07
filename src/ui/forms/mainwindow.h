#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QListWidget>
#include <QMainWindow>
#include "ui/widgets/topbar.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow* instance();
    QListWidget* notificationMenu;
    TopBar* topbar;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void showNotifications();
private slots:
    void returnFromSearch();
    void returnFromWatchHistorySearch();
    void search();
    void searchWatchHistory();
private:
    void browse();
    void resizeEvent(QResizeEvent*) override;
    void tryRestoreData();
    bool doNotBrowse = false;
    QString lastSearchQuery;
    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
