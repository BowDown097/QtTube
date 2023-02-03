#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ui/widgets/findbar.h"
#include "ui/widgets/topbar.h"
#include <QKeyEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static QStackedWidget* centralWidget() { return m_centralWidget; }
    static TopBar* topbar() { return m_topbar; }
public slots:
    void showNotifications();
private slots:
    void returnFromSearch();
    void returnFromWatchHistorySearch();
    void search();
    void searchWatchHistory();
private:
    void browse();
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent*) override;
    void tryRestoreData();

    static inline QStackedWidget* m_centralWidget;
    static inline FindBar* m_findbar;
    static inline TopBar* m_topbar;

    bool doNotBrowse = false;
    QListWidget* notificationMenu;
    QString lastSearchQuery;
    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
