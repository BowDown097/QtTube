#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#ifdef USEMPV
#include <ui/watchview-mpv.h>
#else
#include <ui/watchview-ytp.h>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow* instance();
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void returnFromSearch();
    void search();
    void showSettings();
    void signinClicked();
private:
    bool doNotBrowse = false;
    WatchView* watchView;
    void browse();
    void tryRestoreData();
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
