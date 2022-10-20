#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "topbar.h"
#ifdef USEMPV
#include "watchview-mpv.h"
#else
#include "watchview-ytp.h"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow* instance();
    TopBar* topbar;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void returnFromSearch();
    void search();
private:
    void browse();
    void resizeEvent(QResizeEvent*) override;
    void tryRestoreData();
    bool doNotBrowse = false;
    Ui::MainWindow* ui;
    WatchView* watchView;
};
#endif // MAINWINDOW_H
