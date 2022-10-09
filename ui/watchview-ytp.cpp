#ifndef USEMPV
#include "innertube.hpp"
#include "mainwindow.h"
#include "watchview-shared.h"
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
    MainWindow::instance()->topbar->setVisible(false);
    MainWindow::instance()->topbar->alwaysShow = false;
    connect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);

    titleLabel = new QLabel(this);
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 4));
    titleLabel->setText(player.videoDetails.title);
    titleLabel->setWordWrap(true);

    wePlayer = new WebEnginePlayer(InnerTube::instance().context(), InnerTube::instance().authStore(), player, this);
    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);

    stackedWidget->setCurrentIndex(1);
    titleLabel->move(0, wePlayer->y() + playerSize.height());
    titleLabel->setFixedWidth(playerSize.width());

    wePlayer->play(player.videoDetails.videoId, progress);
    MainWindow::instance()->setWindowTitle(player.videoDetails.title + " - QtTube");
    WatchViewShared::toggleIdleSleep(true);
}

void WatchView::resizeEvent(QResizeEvent*)
{
    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);
    titleLabel->move(0, wePlayer->y() + playerSize.height());
    titleLabel->setFixedWidth(playerSize.width());
}

#endif // USEMPV
