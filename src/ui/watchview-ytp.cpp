#ifndef USEMPV
#include "../settingsstore.h"
#include "innertube.hpp"
#include "mainwindow.h"
#include "watchview-shared.hpp"
#include "watchview-ytp.h"
#include <QApplication>

WatchView* WatchView::instance()
{
    static WatchView* wv = new WatchView;
    return wv;
}

WatchView::WatchView(QWidget* parent) : QWidget(parent) {}

QSize WatchView::calcPlayerSize()
{
    int playerWidth = width();
    int playerHeight = playerWidth * 9/16;
    if (playerHeight > MainWindow::instance()->height() - 125)
    {
        playerHeight = MainWindow::instance()->height() - 125;
        playerWidth = playerHeight * 16/9;
    }

    return QSize(playerWidth, playerHeight);
}

void WatchView::goBack()
{
    MainWindow::instance()->topbar->alwaysShow = true;
    disconnect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);

    titleLabel->deleteLater();
    wePlayer->deleteLater();
    stackedWidget->setCurrentIndex(0);

    WatchViewShared::toggleIdleSleep(false);
}

void WatchView::initialize(QStackedWidget* stackedWidget) { this->stackedWidget = stackedWidget; }

void WatchView::loadVideo(const InnertubeEndpoints::Player& player, int progress)
{
    titleLabel = new QLabel(this);
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 4));
    titleLabel->setText(player.videoDetails.title);
    titleLabel->setWordWrap(true);

    wePlayer = new WebEnginePlayer(InnerTube::instance().context(), InnerTube::instance().authStore(), player,
                                   SettingsStore::instance().playbackTracking, SettingsStore::instance().watchtimeTracking, this);
    stackedWidget->setCurrentIndex(1);
    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);

    titleLabel->move(0, playerSize.height() + 5);
    titleLabel->setFixedWidth(playerSize.width());

    wePlayer->play(player.videoDetails.videoId, progress, SettingsStore::instance().showSBToasts, SettingsStore::instance().sponsorBlockCategories);
    MainWindow::instance()->setWindowTitle(player.videoDetails.title + " - QtTube");
    WatchViewShared::toggleIdleSleep(true);

    MainWindow::instance()->topbar->setVisible(false);
    MainWindow::instance()->topbar->alwaysShow = false;
    connect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);
}

void WatchView::resizeEvent(QResizeEvent*)
{
    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);
    titleLabel->move(0, playerSize.height() + 5);
    titleLabel->setFixedWidth(playerSize.width());
}

#endif // USEMPV
