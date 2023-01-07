#ifndef USEMPV
#include "watchview-ytp.h"
#include "innertube.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include "watchview-shared.h"
#include <QApplication>

WatchView* WatchView::instance()
{
    static WatchView* wv = new WatchView;
    return wv;
}

void WatchView::goBack()
{
    MainWindow::instance()->topbar->alwaysShow = true;
    disconnect(MainWindow::instance()->topbar->logo, &TubeLabel::clicked, this, &WatchView::goBack);
    UIUtilities::clearLayout(pageLayout);
    pageLayout->deleteLater();
    stackedWidget->setCurrentIndex(0);
    WatchViewShared::toggleIdleSleep(false);
}

void WatchView::loadVideo(const QString& videoId, int progress)
{
    InnertubeEndpoints::NextResponse nextResp = InnerTube::instance().get<InnertubeEndpoints::Next>(videoId).response;
    InnertubeEndpoints::PlayerResponse playerResp = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId).response;

    stackedWidget->setCurrentIndex(1);
    QSize playerSize = WatchViewShared::calcPlayerSize(width(), MainWindow::instance()->height());

    pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(5);

    wePlayer = new WebEnginePlayer(this);
    wePlayer->setAuthStore(InnerTube::instance().authStore());
    wePlayer->setContext(InnerTube::instance().context());
    pageLayout->addWidget(wePlayer);

    titleLabel = new TubeLabel(playerResp.videoDetails.title, this);
    titleLabel->setFixedWidth(playerSize.width());
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 4));
    titleLabel->setWordWrap(true);
    pageLayout->addWidget(titleLabel);

    primaryInfoHbox = new QHBoxLayout;
    primaryInfoHbox->setContentsMargins(0, 0, 0, 0);

    channelIcon = new TubeLabel(this);
    channelIcon->setClickable(true, false);
    channelIcon->setMaximumSize(55, 48);
    channelIcon->setMinimumSize(55, 48);
    primaryInfoHbox->addWidget(channelIcon);

    primaryInfoVbox = new QVBoxLayout;

    channelName = new TubeLabel(this);
    channelName->setClickable(true, true);
    channelName->setText(nextResp.secondaryInfo.channelName.text);
    primaryInfoVbox->addWidget(channelName);
    connect(channelName, &TubeLabel::clicked, this, [this, nextResp] {
        disconnect(MainWindow::instance()->topbar->logo, &TubeLabel::clicked, this, &WatchView::goBack);
        WatchViewShared::toggleIdleSleep(false);
        WatchViewShared::navigateChannel(nextResp.secondaryInfo.subscribeButton.channelId);
        UIUtilities::clearLayout(pageLayout);
        pageLayout->deleteLater();
    });

    subscribeHbox = new QHBoxLayout(this);
    subscribeHbox->setContentsMargins(0, 0, 0, 0);
    subscribeHbox->setSpacing(0);

    if (nextResp.secondaryInfo.subscribeButton.enabled)
    {
        subscribeWidget = new SubscribeWidget(nextResp.secondaryInfo.subscribeButton, this);
        subscribeHbox->addWidget(subscribeWidget);
    }

    subscribersLabel = new TubeLabel(this);
    subscribersLabel->setStyleSheet(R"(
    border: 1px solid #333;
    font-size: 11px;
    line-height: 24px;
    padding: 0 6px 0 4.5px;
    border-radius: 2px;
    text-align: center;
    )");
    WatchViewShared::setSubscriberCount(nextResp.secondaryInfo, subscribersLabel);
    subscribeHbox->addWidget(subscribersLabel);

    primaryInfoVbox->addLayout(subscribeHbox);
    primaryInfoHbox->addLayout(primaryInfoVbox);
    primaryInfoHbox->addStretch();

    primaryInfoWrapper = new QWidget(this);
    primaryInfoWrapper->setFixedWidth(playerSize.width());
    primaryInfoWrapper->setLayout(primaryInfoHbox);
    pageLayout->addWidget(primaryInfoWrapper);

    wePlayer->play(playerResp.videoDetails.videoId, progress);
    wePlayer->setFixedSize(playerSize);
    wePlayer->setPlayerResponse(playerResp);

    pageLayout->addStretch(); // disable the layout from stretching on resize

    if (!nextResp.secondaryInfo.channelIcons.isEmpty())
    {
        QList<InnertubeObjects::GenericThumbnail> channelIcons = nextResp.secondaryInfo.channelIcons;
        auto bestThumb = *std::find_if(channelIcons.cbegin(), channelIcons.cend(), [](const InnertubeObjects::GenericThumbnail& t)
        {
            return t.width >= 48;
        });
        HttpReply* reply = Http::instance().get(bestThumb.url);
        connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) { WatchViewShared::setChannelIcon(reply, channelIcon); });
    }

    MainWindow::instance()->setWindowTitle(playerResp.videoDetails.title + " - QtTube");
    WatchViewShared::toggleIdleSleep(true);

    MainWindow::instance()->topbar->setVisible(false);
    MainWindow::instance()->topbar->alwaysShow = false;
    connect(MainWindow::instance()->topbar->logo, &TubeLabel::clicked, this, &WatchView::goBack);
}

void WatchView::resizeEvent(QResizeEvent*)
{
    if (!primaryInfoWrapper) return;

    QSize playerSize = WatchViewShared::calcPlayerSize(width(), MainWindow::instance()->height());
    primaryInfoWrapper->setFixedWidth(playerSize.width());
    titleLabel->setFixedWidth(playerSize.width());
    wePlayer->setFixedSize(playerSize);
}
#endif // USEMPV
