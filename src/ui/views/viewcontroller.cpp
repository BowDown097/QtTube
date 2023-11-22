#include "viewcontroller.h"
#include "channelview.h"
#include "innertube/innertubeexception.h"
#include "mainwindow.h"
#include "watchview.h"
#include <QMessageBox>

void ViewController::loadChannel(const QString& channelId)
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
            MainWindow::topbar()->alwaysShow = true;
        }
    }

    MainWindow::centralWidget()->addWidget(channelView);
    MainWindow::centralWidget()->setCurrentWidget(channelView);
    QObject::connect(MainWindow::topbar()->logo, &TubeLabel::clicked, channelView, [channelView]
    {
        channelView->deleteLater();
        MainWindow::topbar()->alwaysShow = true;
    });
}

void ViewController::loadVideo(const QString& videoId, int progress)
{
    if (WatchView* casted = qobject_cast<WatchView*>(MainWindow::centralWidget()->currentWidget()))
    {
        casted->hotLoadVideo(videoId, progress);
        return;
    }
    else if (ChannelView* channelView = qobject_cast<ChannelView*>(MainWindow::centralWidget()->currentWidget()))
    {
        channelView->deleteLater();
    }

    WatchView* watchView = new WatchView(videoId, progress);
    MainWindow::centralWidget()->addWidget(watchView);
    MainWindow::centralWidget()->setCurrentWidget(watchView);

    QObject::connect(watchView, &WatchView::navigateChannelRequested, std::bind(&ViewController::loadChannel, std::placeholders::_1));
    QObject::connect(MainWindow::topbar()->logo, &TubeLabel::clicked, watchView, [watchView]
    {
        watchView->deleteLater();
        MainWindow::topbar()->alwaysShow = true;
    });
    QObject::connect(watchView, &WatchView::loadFailed, [watchView](const InnertubeException& ie)
    {
        QMessageBox::critical(nullptr, "Failed to load video", ie.message());
        watchView->deleteLater();
        MainWindow::topbar()->alwaysShow = true;
    });
}
