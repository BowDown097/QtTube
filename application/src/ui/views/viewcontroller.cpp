#include "viewcontroller.h"
#include "channelview.h"
#include "mainwindow.h"
#include "ui/widgets/topbar/topbar.h"
#include "utils/uiutils.h"
#include "watchview.h"
#include <QMessageBox>
#include <QStackedWidget>

namespace ViewController
{
    void loadChannel(const QString& channelId, PluginEntry* plugin)
    {
        MainWindow* mainWindow = UIUtils::getMainWindow();
        if (ChannelView* casted = qobject_cast<ChannelView*>(mainWindow->centralWidget()->currentWidget()))
        {
            casted->hotLoadChannel(channelId);
            return;
        }
        else if (WatchView* watchView = qobject_cast<WatchView*>(mainWindow->centralWidget()->currentWidget()))
        {
            watchView->deleteLater();
        }

        ChannelView* channelView = new ChannelView(channelId, plugin);
        mainWindow->centralWidget()->addWidget(channelView);
        mainWindow->centralWidget()->setCurrentWidget(channelView);

        QObject::connect(mainWindow->topbar()->logo, &TubeLabel::clicked, channelView, [=] {
            channelView->deleteLater();
            mainWindow->topbar()->setAlwaysShow(true);
        });
        QObject::connect(channelView, &ChannelView::loadFailed, channelView, [=](const QtTubePlugin::Exception& ex) {
            QMessageBox::critical(nullptr, "Failed to Load Channel", ex.message());
            channelView->deleteLater();
            mainWindow->topbar()->setAlwaysShow(true);
            mainWindow->topbar()->show();
        });
    }

    void loadVideo(const QString& videoId, PluginEntry* plugin, int progress,
                   PreloadData::WatchView* preload, bool continuePlayback)
    {
        MainWindow* mainWindow = UIUtils::getMainWindow();
        if (WatchView* casted = qobject_cast<WatchView*>(mainWindow->centralWidget()->currentWidget()))
        {
            casted->hotLoadVideo(videoId, progress, preload, continuePlayback);
            return;
        }
        else if (ChannelView* channelView = qobject_cast<ChannelView*>(mainWindow->centralWidget()->currentWidget()))
        {
            channelView->deleteLater();
        }

        WatchView* watchView = new WatchView(videoId, plugin, progress, preload);
        mainWindow->centralWidget()->addWidget(watchView);
        mainWindow->centralWidget()->setCurrentWidget(watchView);

        QObject::connect(mainWindow->topbar()->logo, &TubeLabel::clicked, watchView, [=] {
            watchView->deleteLater();
            mainWindow->topbar()->setAlwaysShow(true);
        });
        QObject::connect(watchView, &WatchView::loadFailed, watchView, [=](const QtTubePlugin::Exception& ex) {
            QMessageBox::critical(nullptr, "Failed to Load Video", ex.message());
            watchView->deleteLater();
            mainWindow->topbar()->setAlwaysShow(true);
            mainWindow->topbar()->show();
        });
    }

    void unloadCurrent()
    {
        MainWindow* mainWindow = UIUtils::getMainWindow();
        if (ChannelView* channelView = qobject_cast<ChannelView*>(mainWindow->centralWidget()->currentWidget()))
            channelView->deleteLater();
        else if (WatchView* watchView = qobject_cast<WatchView*>(mainWindow->centralWidget()->currentWidget()))
            watchView->deleteLater();
    }
}
