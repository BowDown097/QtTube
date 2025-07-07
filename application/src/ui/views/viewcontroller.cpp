#include "viewcontroller.h"
#include "channelview.h"
#include "mainwindow.h"
#include "watchview.h"
#include <QMessageBox>

namespace ViewController
{
    void loadChannel(const QString& channelId)
    {
        if (ChannelView* casted = qobject_cast<ChannelView*>(MainWindow::centralWidget()->currentWidget()))
        {
            casted->hotLoadChannel(channelId);
            return;
        }
        else if (WatchView* watchView = qobject_cast<WatchView*>(MainWindow::centralWidget()->currentWidget()))
        {
            watchView->deleteLater();
        }

        ChannelView* channelView = new ChannelView(channelId);
        MainWindow::centralWidget()->addWidget(channelView);
        MainWindow::centralWidget()->setCurrentWidget(channelView);

        QObject::connect(MainWindow::topbar()->logo, &TubeLabel::clicked, channelView, [channelView] {
            channelView->deleteLater();
            MainWindow::topbar()->setAlwaysShow(true);
        });
        QObject::connect(channelView, &ChannelView::loadFailed, [channelView](const QtTubePlugin::Exception& ex) {
            QMessageBox::critical(nullptr, "Failed to load channel", ex.message());
            channelView->deleteLater();
            MainWindow::topbar()->setAlwaysShow(true);
            MainWindow::topbar()->show();
        });
    }

    void loadVideo(const QString& videoId, int progress, PreloadData::WatchView* preload, bool continuePlayback)
    {
        if (WatchView* casted = qobject_cast<WatchView*>(MainWindow::centralWidget()->currentWidget()))
        {
            casted->hotLoadVideo(videoId, progress, preload, continuePlayback);
            return;
        }
        else if (ChannelView* channelView = qobject_cast<ChannelView*>(MainWindow::centralWidget()->currentWidget()))
        {
            channelView->deleteLater();
        }

        WatchView* watchView = new WatchView(videoId, progress, preload);
        MainWindow::centralWidget()->addWidget(watchView);
        MainWindow::centralWidget()->setCurrentWidget(watchView);

        QObject::connect(MainWindow::topbar()->logo, &TubeLabel::clicked, watchView, [watchView] {
            watchView->deleteLater();
            MainWindow::topbar()->setAlwaysShow(true);
        });
        QObject::connect(watchView, &WatchView::loadFailed, [watchView](const QtTubePlugin::Exception& ex) {
            QMessageBox::critical(nullptr, "Failed to load video", ex.message());
            watchView->deleteLater();
            MainWindow::topbar()->setAlwaysShow(true);
            MainWindow::topbar()->show();
        });
    }

    void unloadCurrent()
    {
        if (ChannelView* channelView = qobject_cast<ChannelView*>(MainWindow::centralWidget()->currentWidget()))
            channelView->deleteLater();
        else if (WatchView* watchView = qobject_cast<WatchView*>(MainWindow::centralWidget()->currentWidget()))
            watchView->deleteLater();
    }
}
