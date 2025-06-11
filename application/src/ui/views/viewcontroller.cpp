#include "viewcontroller.h"
#include "channelview.h"
#include "innertube/innertubeexception.h"
#include "mainwindow.h"
#include "watchview.h"
#include <QMessageBox>

namespace ViewController
{
    void loadChannel(const QString& channelId)
    {
        if (ChannelView* casted = qobject_cast<ChannelView*>(MainWindow::centralWidget()->currentWidget()))
        {
            try
            {
                casted->hotLoadChannel(channelId);
            }
            catch (InnertubeException& ie)
            {
                QMessageBox::critical(nullptr, "Failed to load channel", ie.message());
            }

            return;
        }
        else if (WatchView* watchView = qobject_cast<WatchView*>(MainWindow::centralWidget()->currentWidget()))
        {
            watchView->deleteLater();
        }

        ChannelView* channelView = nullptr;
        try
        {
            channelView = new ChannelView(channelId);
        }
        catch (InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to load channel", ie.message());
            if (channelView)
            {
                channelView->deleteLater();
                MainWindow::topbar()->setAlwaysShow(true);
            }
        }

        MainWindow::centralWidget()->addWidget(channelView);
        MainWindow::centralWidget()->setCurrentWidget(channelView);
        QObject::connect(MainWindow::topbar()->logo, &TubeLabel::clicked, channelView, [channelView]
        {
            channelView->deleteLater();
            MainWindow::topbar()->setAlwaysShow(true);
        });
    }

    void loadVideo(const QString& videoId, int progress, PreloadData::WatchView* preload)
    {
        if (WatchView* casted = qobject_cast<WatchView*>(MainWindow::centralWidget()->currentWidget()))
        {
            casted->hotLoadVideo(videoId, progress, preload);
            return;
        }
        else if (ChannelView* channelView = qobject_cast<ChannelView*>(MainWindow::centralWidget()->currentWidget()))
        {
            channelView->deleteLater();
        }

        WatchView* watchView = new WatchView(videoId, progress, preload);
        MainWindow::centralWidget()->addWidget(watchView);
        MainWindow::centralWidget()->setCurrentWidget(watchView);

        QObject::connect(MainWindow::topbar()->logo, &TubeLabel::clicked, watchView, [watchView]
        {
            watchView->deleteLater();
            MainWindow::topbar()->setAlwaysShow(true);
        });
        QObject::connect(watchView, &WatchView::loadFailed, [watchView](const QtTube::PluginException& ex)
        {
            QMessageBox::critical(nullptr, "Failed to load video", ex.message());
            watchView->deleteLater();
            MainWindow::topbar()->setAlwaysShow(true);
            MainWindow::topbar()->show();
        });
    }
}
