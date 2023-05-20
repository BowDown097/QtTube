#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ui/widgets/findbar.h"
#include "ui/widgets/topbar/topbar.h"
#include <QKeyEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QResizeEvent>
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
    static QSize size() { return m_size; }
    static TopBar* topbar() { return m_topbar; }
    static WId windowId() { return m_winId; }
public slots:
    void showNotifications();
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void returnFromSearch();
    void returnFromWatchHistorySearch();
    void search();
    void searchWatchHistory();
private:
    void browse();
    void tryRestoreData();

    static inline QStackedWidget* m_centralWidget;
    static inline QSize m_size;
    static inline TopBar* m_topbar;
    static inline WId m_winId;

    bool doNotBrowse = false;
    FindBar* findbar;
    QListWidget* notificationMenu;
    QString lastSearchQuery;
    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
