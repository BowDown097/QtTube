#ifndef USEMPV
#include "innertube.hpp"
#include "mainwindow.h"
#include "watchview-ytp.h"
#include <QApplication>
#include <QTimer>

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
    backButton->deleteLater();
    titleLabel->deleteLater();
    wePlayer->deleteLater();
    stackedWidget->setCurrentIndex(0);
}

void WatchView::initialize(QStackedWidget* stackedWidget) { this->stackedWidget = stackedWidget; }

void WatchView::loadVideo(const InnertubeEndpoints::Player& player)
{
    backButton = new QPushButton(this);
    backButton->setFixedWidth(24);
    backButton->setText("⬅");
    connect(backButton, &QPushButton::clicked, this, &WatchView::goBack);

    titleLabel = new QLabel(this);
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 4));
    titleLabel->setText(player.videoDetails.title);
    titleLabel->setWordWrap(true);

    wePlayer = new WebEnginePlayer(InnerTube::instance().context(), InnerTube::instance().authStore(), player, this);
    wePlayer->move(0, backButton->height() + 5);

    stackedWidget->setCurrentIndex(1);

    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);
    titleLabel->move(0, wePlayer->y() + playerSize.height());
    titleLabel->setFixedWidth(playerSize.width());

    wePlayer->play(player.videoDetails.videoId);
    MainWindow::instance()->setWindowTitle(player.videoDetails.title + " - youtube-qt");
}

void WatchView::resizeEvent(QResizeEvent*)
{
    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);
    titleLabel->move(0, wePlayer->y() + playerSize.height());
    titleLabel->setFixedWidth(playerSize.width());
}

#endif // USEMPV
