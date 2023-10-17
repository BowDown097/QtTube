#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ui/widgets/continuablelistwidget.h"
#include "ui/widgets/findbar.h"
#include "ui/widgets/topbar/topbar.h"
#include <QCommandLineParser>
#include <QKeyEvent>
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
    explicit MainWindow(const QCommandLineParser& parser, QWidget* parent = nullptr);
    ~MainWindow();

    static QStackedWidget* centralWidget() { return m_centralWidget; }
    static QSize size() { return m_size; }
    static TopBar* topbar() { return m_topbar; }
#ifdef Q_OS_WIN
    static WId windowId() { return m_winId; }
#endif
public slots:
    void showAccountMenu();
    void showNotifications();
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void performFilteredSearch();
    void reloadCurrentTab();
    void returnFromSearch();
    void returnFromWatchHistorySearch();
    void search(const QString& query, SearchBox::SearchType searchType);
    void searchWatchHistory();
private:
    void browse();
    void searchByLink(const QString& link);
    void searchByQuery(const QString& query);
    void tryRestoreData();
    void trySwitchGridStatus(QListWidget* listWidget);

    static inline QStackedWidget* m_centralWidget;
    static inline QSize m_size;
    static inline TopBar* m_topbar;
#ifdef Q_OS_WIN
    static inline WId m_winId;
#endif

    bool doNotBrowse = false;
    FindBar* findbar;
    QString lastSearchQuery;
    ContinuableListWidget* notificationMenu;
    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
