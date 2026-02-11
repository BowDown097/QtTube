#pragma once
#include "ui/widgets/topbar/searchbox.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ContinuableListWidget;
class FindBar;
class QCommandLineParser;
class QKeyEvent;
class QResizeEvent;
class QStackedWidget;
class TopBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QCommandLineParser& parser, QWidget* parent = nullptr);
    ~MainWindow();

    QStackedWidget* centralWidget();
    TopBar* topbar();
public slots:
    void showAccountMenu();
    void showNotifications();
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void activePluginChanged(struct PluginData* activePlugin);
    void reloadCurrentTab();
    void returnFromSearch();
    void returnFromWatchHistorySearch();
    void search(const QString& query, SearchBox::SearchType searchType);
    void searchWatchHistory();
private:
    void browse();
    void searchByQuery(const QString& query);

    bool m_doNotBrowse = false;
    FindBar* m_findbar;
    QString m_lastSearchQuery;
    ContinuableListWidget* m_notificationMenu;
    TopBar* m_topbar;
    Ui::MainWindow* ui;
};
