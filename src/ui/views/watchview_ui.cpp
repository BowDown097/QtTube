#include "watchview_ui.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include "ui/widgets/watchnextfeed.h"
#include "watchviewplayer.h"
#include <QBoxLayout>
#include <QProgressBar>
#include <QScrollArea>
#include <QScrollBar>

constexpr QLatin1String LikeBarStylesheet(R"(
    QProgressBar {
        border-radius: 2px;
        background-color: #606060;
    }

    QProgressBar::chunk {
        background-color: #1879c6;
    }
)");

void WatchView_Ui::moveFeed(WatchViewPlayer::ScaleMode scaleMode)
{
    feed->currentList()->verticalScrollBar()->setEnabled(scaleMode == WatchViewPlayer::ScaleMode::Scaled);
    if (scaleMode == WatchViewPlayer::ScaleMode::NoScale)
    {
        primaryLayout->removeWidget(feed);
        frameLayout->addWidget(feed);
        feed->setMinimumHeight(500);
    }
    else if (scaleMode == WatchViewPlayer::ScaleMode::Scaled)
    {
        frameLayout->removeWidget(feed);
        primaryLayout->addWidget(feed);
        feed->setMinimumHeight(0);
    }
}

void WatchView_Ui::setupUi(QWidget* watchView)
{
    setupFrame(watchView);
    setupPlayer(watchView);
    setupTitle(watchView);
    setupPrimaryInfo(watchView);
    setupMenu(watchView);
    setupDate(watchView);
    setupDescription(watchView);
    setupFeed(watchView);
    frameLayout->addStretch();
}

void WatchView_Ui::setupDate(QWidget* watchView)
{
    infoSpacer = new QSpacerItem(20, 20);
    frameLayout->addItem(infoSpacer);

    date = new TubeLabel(watchView);
    date->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
    date->setTextFormat(Qt::RichText);
    date->setWordWrap(true);

    frameLayout->addWidget(date);
}

void WatchView_Ui::setupDescription(QWidget* watchView)
{
    description = new TubeLabel(watchView);
    description->setFixedWidth(player->size().width());
    description->setMaximumLines(3);
    description->setTextFormat(Qt::RichText);
    description->setWordWrap(true);
    frameLayout->addWidget(description);

    showMoreLabel = new TubeLabel(watchView);
    showMoreLabel->setAlignment(Qt::AlignCenter);
    showMoreLabel->setClickable(true);
    showMoreLabel->setFixedWidth(player->size().width());
    showMoreLabel->setStyleSheet("border-top: 1px solid " + qApp->palette().text().color().name());
    showMoreLabel->setText("SHOW MORE");
    frameLayout->addWidget(showMoreLabel);
    connect(showMoreLabel, &TubeLabel::clicked, this, &WatchView_Ui::toggleShowMore);
}

void WatchView_Ui::setupFeed(QWidget* watchView)
{
    feed = new WatchNextFeed(watchView);
    if (player->scaleMode() == WatchViewPlayer::ScaleMode::NoScale)
    {
        frameLayout->addWidget(feed);
        feed->currentList()->verticalScrollBar()->setEnabled(false);
        feed->setMinimumHeight(500);
    }
    else if (player->scaleMode() == WatchViewPlayer::ScaleMode::Scaled)
    {
        primaryLayout->addWidget(feed);
        feed->setMaximumWidth(MainWindow::size().width() - player->size().width() - primaryLayout->spacing());
    }
}

void WatchView_Ui::setupFrame(QWidget* watchView)
{
    scrollArea = new QScrollArea(watchView);
    scrollArea->setMaximumHeight(MainWindow::size().height());
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->show();
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &WatchView_Ui::scrollValueChanged);

    primaryLayout = new QHBoxLayout(watchView);
    primaryLayout->setSpacing(5);

    if (qtTubeApp->settings().autoHideTopBar)
        primaryLayout->setContentsMargins(0, 0, 0, 0);
    else
        primaryLayout->setContentsMargins(0, MainWindow::topbar()->height(), 0, 0);

    frame = new QFrame(scrollArea);
    frameLayout = new QVBoxLayout;
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(frameLayout);

    scrollArea->setWidget(frame);
    primaryLayout->addWidget(scrollArea);
}

void WatchView_Ui::setupMenu(QWidget* watchView)
{
    menuWrapper = new QWidget(watchView);
    menuWrapper->setFixedWidth(player->size().width());
    frameLayout->addWidget(menuWrapper);

    menuVbox = new QVBoxLayout(menuWrapper);
    menuVbox->setContentsMargins(0, 0, 20, 0);
    menuVbox->setSpacing(3);

    viewCount = new TubeLabel(watchView);
    viewCount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    viewCount->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 3));
    menuVbox->addWidget(viewCount);

    if (qtTubeApp->settings().returnDislikes)
    {
        // i have to wrap the like bar for alignment to work... cringe!
        likeBarWrapper = new QHBoxLayout;
        likeBarWrapper->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        likeBar = new QProgressBar(watchView);
        likeBar->hide();
        likeBar->setFixedSize(155, 2);
        likeBar->setStyleSheet(LikeBarStylesheet);
        likeBarWrapper->addWidget(likeBar);

        menuVbox->addLayout(likeBarWrapper);
    }

    topLevelButtons = new QHBoxLayout;
    topLevelButtons->setContentsMargins(0, 3, 0, 0);
    topLevelButtons->setSpacing(0);
    menuVbox->addLayout(topLevelButtons);
}

void WatchView_Ui::setupPlayer(QWidget* watchView)
{
    player = new WatchViewPlayer(watchView, MainWindow::size());
    scrollArea->setMaximumWidth(player->size().width());
    frameLayout->addWidget(player->widget());
    connect(player, &WatchViewPlayer::scaleModeChanged, this, &WatchView_Ui::moveFeed);
}

void WatchView_Ui::setupPrimaryInfo(QWidget* watchView)
{
    primaryInfoWrapper = new QWidget(watchView);
    primaryInfoWrapper->setFixedWidth(player->size().width());
    frameLayout->addWidget(primaryInfoWrapper);

    primaryInfoHbox = new QHBoxLayout(primaryInfoWrapper);
    primaryInfoHbox->setContentsMargins(0, 0, 0, 0);

    channelIcon = new TubeLabel(watchView);
    channelIcon->setClickable(true);
    channelIcon->setFixedSize(48, 48);
    channelIcon->setScaledContents(true);
    primaryInfoHbox->addWidget(channelIcon);
    primaryInfoHbox->addSpacerItem(new QSpacerItem(7, 0));

    primaryInfoVbox = new QVBoxLayout;

    channelLabel = new ChannelLabel(watchView);
    channelLabel->text->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
    channelLabel->addStretch();
    primaryInfoVbox->addWidget(channelLabel);

    subscribeWidget = new SubscribeWidget(watchView);
    subscribeWidget->layout->addStretch();
    subscribeWidget->subscribersCountLabel->hide();
    primaryInfoVbox->addWidget(subscribeWidget);

    primaryInfoHbox->addLayout(primaryInfoVbox);
    primaryInfoHbox->addStretch();
}

void WatchView_Ui::setupTitle(QWidget* watchView)
{
    titleLabel = new TubeLabel(watchView);
    titleLabel->setFixedWidth(player->size().width());
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 4));
    titleLabel->setWordWrap(true);
    frameLayout->addWidget(titleLabel);
}

void WatchView_Ui::scrollValueChanged(int value)
{
    if (player->scaleMode() == WatchViewPlayer::ScaleMode::Scaled)
        return;

    QScrollBar* feedScrollBar = feed->currentList()->verticalScrollBar();
    if (value == scrollArea->verticalScrollBar()->maximum())
        feedScrollBar->setEnabled(true);
    else if (feedScrollBar->value() == feedScrollBar->minimum())
        feedScrollBar->setEnabled(false);
}

void WatchView_Ui::toggleShowMore()
{
    if (showMoreLabel->text() == "SHOW MORE")
    {
        description->setMaximumHeight(QWIDGETSIZE_MAX);
        description->setMaximumLines(-1);
        showMoreLabel->setText("SHOW LESS");
    }
    else
    {
        description->setMaximumLines(3);
        showMoreLabel->setText("SHOW MORE");
    }
}
