#ifndef USEMPV
#include "../settingsstore.h"
#include "http.h"
#include "innertube.h"
#include "mainwindow.h"
#include "uiutilities.h"
#include "watchview-shared.hpp"
#include "watchview-ytp.h"
#include <QApplication>

WatchView* WatchView::instance()
{
    static WatchView* wv = new WatchView;
    return wv;
}

void WatchView::goBack()
{
    MainWindow::instance()->topbar->alwaysShow = true;
    disconnect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);
    UIUtilities::clearLayout(pageLayout);
    pageLayout->deleteLater();
    stackedWidget->setCurrentIndex(0);
    WatchViewShared::toggleIdleSleep(false);
}

void WatchView::loadVideo(const InnertubeEndpoints::NextResponse& nextResp, const InnertubeEndpoints::PlayerResponse& playerResp, int progress)
{
    stackedWidget->setCurrentIndex(1);

    pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(5);

    wePlayer = new WebEnginePlayer(this);
    wePlayer->setAuthStore(InnerTube::instance().authStore());
    wePlayer->setContext(InnerTube::instance().context());
    wePlayer->setUsePlaybackTracking(SettingsStore::instance().playbackTracking);
    wePlayer->setUseWatchtimeTracking(SettingsStore::instance().watchtimeTracking);
    pageLayout->addWidget(wePlayer);

    titleLabel = new QLabel(this);
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 4));
    titleLabel->setText(playerResp.videoDetails.title);
    titleLabel->setWordWrap(true);
    pageLayout->addWidget(titleLabel);

    primaryInfoHbox = new QHBoxLayout;
    primaryInfoHbox->setContentsMargins(0, 0, 0, 0);
    primaryInfoHbox->setSizeConstraint(QBoxLayout::SizeConstraint::SetFixedSize);

    channelIcon = new ClickableLabel(false, this);
    channelIcon->setMaximumSize(55, 48);
    channelIcon->setMinimumSize(55, 48);
    primaryInfoHbox->addWidget(channelIcon);

    primaryInfoVbox = new QVBoxLayout;

    channelName = new ClickableLabel(true, this);
    channelName->setText(nextResp.secondaryInfo.channelName.text);
    primaryInfoVbox->addWidget(channelName);

    subscribersLabel = new QLabel(this);
    setSubscriberCount(nextResp.secondaryInfo);
    primaryInfoVbox->addWidget(subscribersLabel);

    primaryInfoHbox->addLayout(primaryInfoVbox);
    pageLayout->addLayout(primaryInfoHbox);

    wePlayer->play(playerResp.videoDetails.videoId, progress, SettingsStore::instance().showSBToasts, SettingsStore::instance().sponsorBlockCategories);
    wePlayer->setFixedSize(calcPlayerSize());
    wePlayer->setPlayerResponse(playerResp);

    pageLayout->addStretch(); // disable the layout from stretching on resize

    if (!nextResp.secondaryInfo.channelIcons.isEmpty())
    {
        auto bestThumb = *std::ranges::find_if(nextResp.secondaryInfo.channelIcons, [](const auto& t) { return t.width >= 48; });
        HttpReply* reply = Http::instance().get(bestThumb.url);
        connect(reply, &HttpReply::finished, this, &WatchView::setChannelIcon);
    }

    MainWindow::instance()->setWindowTitle(playerResp.videoDetails.title + " - QtTube");
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
