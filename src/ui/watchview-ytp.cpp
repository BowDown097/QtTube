#ifndef USEMPV
#include "../settingsstore.h"
#include "http.h"
#include "innertube.h"
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

void WatchView::goBack()
{
    MainWindow::instance()->topbar->alwaysShow = true;
    disconnect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);

    titleLabel->deleteLater();
    wePlayer->deleteLater();
    primaryInfoWidget->deleteLater();
    stackedWidget->setCurrentIndex(0);

    WatchViewShared::toggleIdleSleep(false);
}

void WatchView::initialize(QStackedWidget* stackedWidget) { this->stackedWidget = stackedWidget; }

void WatchView::loadVideo(const InnertubeEndpoints::Next& next, const InnertubeEndpoints::Player& player, int progress)
{
    titleLabel = new QLabel(this);
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 4));
    titleLabel->setText(player.videoDetails.title);
    titleLabel->setWordWrap(true);

    primaryInfoWidget = new QWidget(this);

    primaryInfoHbox = new QHBoxLayout(primaryInfoWidget);
    primaryInfoHbox->setSpacing(0);
    primaryInfoHbox->setContentsMargins(0, 0, 0, 0);

    channelIcon = new ClickableLabel(false, primaryInfoWidget);
    channelIcon->setMaximumSize(55, 48);
    channelIcon->setMinimumSize(55, 48);
    primaryInfoHbox->addWidget(channelIcon);

    hboxVbox = new QVBoxLayout;

    channelName = new ClickableLabel(true, primaryInfoWidget);
    channelName->setText(next.secondaryInfo.channelName.text);
    hboxVbox->addWidget(channelName);

    subscribersLabel = new QLabel(primaryInfoWidget);
    setSubscriberCount(next.secondaryInfo);
    hboxVbox->addWidget(subscribersLabel);

    primaryInfoHbox->addLayout(hboxVbox);

    wePlayer = new WebEnginePlayer(InnerTube::instance().context(), InnerTube::instance().authStore(), player,
                                   SettingsStore::instance().playbackTracking, SettingsStore::instance().watchtimeTracking, this);

    auto bestThumb = *std::ranges::find_if(next.secondaryInfo.channelIcons, [](const auto& t) { return t.width >= 48; });
    HttpReply* reply = Http::instance().get(bestThumb.url);
    connect(reply, &HttpReply::finished, this, &WatchView::setChannelIcon);

    stackedWidget->setCurrentIndex(1);

    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);

    titleLabel->move(0, playerSize.height() + 5);
    titleLabel->setFixedWidth(playerSize.width());
    primaryInfoWidget->move(0, titleLabel->y() + titleLabel->height() + 5);
    primaryInfoWidget->setFixedWidth(playerSize.width());

    wePlayer->play(player.videoDetails.videoId, progress, SettingsStore::instance().showSBToasts, SettingsStore::instance().sponsorBlockCategories);
    MainWindow::instance()->setWindowTitle(player.videoDetails.title + " - QtTube");
    WatchViewShared::toggleIdleSleep(true);

    MainWindow::instance()->topbar->setVisible(false);
    MainWindow::instance()->topbar->alwaysShow = false;
    connect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);
}

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

void WatchView::resizeEvent(QResizeEvent*)
{
    QSize playerSize = calcPlayerSize();
    wePlayer->setFixedSize(playerSize);

    titleLabel->move(0, playerSize.height() + 5);
    titleLabel->setFixedWidth(playerSize.width());

    primaryInfoWidget->move(0, titleLabel->y() + titleLabel->height() + 5);
    primaryInfoWidget->setFixedWidth(playerSize.width());
}

void WatchView::setChannelIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WatchView::setSubscriberCount(const InnertubeObjects::VideoSecondaryInfo& secondaryInfo)
{
    if (!SettingsStore::instance().fullSubs)
    {
        subscribersLabel->setText(secondaryInfo.subscriberCountText.text);
        return;
    }

    Http http;
    http.setReadTimeout(2000);
    http.setMaxRetries(5);

    // have to catch errors here because this API really, REALLY likes to stop working
    HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + secondaryInfo.channelId));
    connect(reply, &HttpReply::error, this, [this, &secondaryInfo] { subscribersLabel->setText(secondaryInfo.subscriberCountText.text); });
    connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
        int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
        subscribersLabel->setText(QLocale::system().toString(subs) + " subscribers");
    });
}

#endif // USEMPV
