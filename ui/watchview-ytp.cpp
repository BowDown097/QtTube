#ifndef USEMPV
#include "mainwindow.h"
#include "watchview-ytp.h"
#include <QPushButton>

WatchView* WatchView::instance()
{
    static WatchView* wv = new WatchView;
    return wv;
}

WatchView::WatchView(QWidget* parent) : QWidget(parent) {}

void WatchView::goBack()
{
    backButton->deleteLater();
    wePlayer->deleteLater();
    stackedWidget->setCurrentIndex(0);
}

void WatchView::initialize(QStackedWidget* stackedWidget)
{
    this->stackedWidget = stackedWidget;
}

void WatchView::loadVideo(const InnertubeEndpoints::Player& player)
{
    backButton = new QPushButton(this);
    backButton->setFixedWidth(24);
    backButton->setText("⬅");
    connect(backButton, &QPushButton::clicked, this, &WatchView::goBack);

    wePlayer = new WebEnginePlayer(this);
    wePlayer->move(0, backButton->height() + 5);

    stackedWidget->setCurrentIndex(1);

    int playerWidth = width();
    int playerHeight = playerWidth * 9/16;
    if (playerHeight > MainWindow::instance()->height() - 125)
    {
        playerHeight = MainWindow::instance()->height() - 125;
        playerWidth = playerHeight * 16/9;
    }

    wePlayer->setFixedSize(playerWidth, playerHeight);
    wePlayer->play(player.videoDetails.videoId);

    MainWindow::instance()->setWindowTitle(player.videoDetails.title + " - youtube-qt");
}

#endif // USEMPV
