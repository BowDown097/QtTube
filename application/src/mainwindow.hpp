#pragma once
#include "plugins/pluginentry.hpp"
#include "plugins/releasedata.hpp"
#include "ui/widgets/topbar/searchbox.hpp"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ContinuableListWidget;
class FindBar;
class QKeyEvent;
class QResizeEvent;
class QStackedWidget;
class TopBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    QStackedWidget* centralWidget();
    TopBar* topbar();

    void reportJsException(const QString& str);
public slots:
    void toggleAccountMenu();
    void toggleNotificationMenu();
protected:
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void activePluginChanged(PluginEntry* activePlugin);
    void authFailed();
    void authSucceeded();
    void pluginUpdateAvailable(const QString& name, const ReleaseData& data);
    void reloadCurrentTab();
    void returnFromSearch();
    void returnFromWatchHistorySearch();
    void search(const QString& query, SearchBox::SearchType searchType);
    void searchWatchHistory();
private:
    void browse();
    void searchByQuery(const QString& query);
    void toggleTabsForActivePlugin();

    PluginEntry* m_activePlugin{};
    FindBar* m_findbar;
    int m_lastBrowseTab{};
    QString m_lastSearchQuery;
    ContinuableListWidget* m_notificationMenu;
    TopBar* m_topbar;
    Ui::MainWindow* ui;
};
