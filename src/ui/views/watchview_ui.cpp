#include "watchview_ui.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include <QApplication>

#ifdef USEMPV
#include "lib/media/mpv/mediampv.h"
#include <QMessageBox>
#endif

void WatchView_Ui::setupUi(QWidget* watchView)
{
    const QSize playerSize = calcPlayerSize(MainWindow::size());

    setupFrame(watchView);
    setupPlayer(watchView, playerSize);
    setupTitle(watchView, playerSize);
    setupPrimaryInfo(watchView, playerSize);
    setupMenu(watchView, playerSize);
    setupDate(watchView);
    setupDescription(watchView, playerSize);

    qobject_cast<QVBoxLayout*>(frame->layout())->addStretch(); // disable the layout from stretching on resize
}

void WatchView_Ui::setupDate(QWidget* watchView)
{
    infoSpacer = new QSpacerItem(20, 20);
    frame->layout()->addItem(infoSpacer);

    date = new TubeLabel(watchView);
    date->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
    frame->layout()->addWidget(date);
}

void WatchView_Ui::setupDescription(QWidget* watchView, const QSize& playerSize)
{
    description = new TubeLabel(watchView);
    description->setFixedWidth(playerSize.width());
    description->setTextFormat(Qt::RichText);
    description->setWordWrap(true);
    UIUtilities::setMaximumLines(description, 3);
    frame->layout()->addWidget(description);

    showMoreLabel = new TubeLabel(watchView);
    showMoreLabel->setAlignment(Qt::AlignCenter);
    showMoreLabel->setClickable(true, false);
    showMoreLabel->setFixedWidth(playerSize.width());
    showMoreLabel->setStyleSheet("border-top: 1px solid " + qApp->palette().text().color().name());
    showMoreLabel->setText("SHOW MORE");
    frame->layout()->addWidget(showMoreLabel);
    connect(showMoreLabel, &TubeLabel::clicked, this, &WatchView_Ui::toggleShowMore);
}

void WatchView_Ui::setupFrame(QWidget* watchView)
{
    scrollArea = new QScrollArea(watchView);
    scrollArea->setFixedSize(MainWindow::size());
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->show();

    frame = new QFrame(scrollArea);
    frame->setLayout(new QVBoxLayout);
    frame->layout()->setContentsMargins(0, 0, 0, 0);
    frame->layout()->setSpacing(5);
    scrollArea->setWidget(frame);
}

void WatchView_Ui::setupMenu(QWidget* watchView, const QSize& playerSize)
{
    menuVbox = new QVBoxLayout(watchView);
    menuVbox->setContentsMargins(0, 0, 20, 0);
    menuVbox->setSpacing(3);

    viewCount = new TubeLabel(watchView);
    viewCount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    viewCount->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 3));
    menuVbox->addWidget(viewCount);

    if (SettingsStore::instance().returnDislikes)
    {
        // i have to wrap the like bar for alignment to work... cringe!
        likeBarWrapper = new QHBoxLayout(watchView);
        likeBarWrapper->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        likeBar = new QProgressBar(watchView);
        likeBar->setFixedSize(155, 2);
        likeBar->setStyleSheet(likeBarStyle);
        likeBar->setTextVisible(false);
        likeBar->setVisible(false);
        likeBarWrapper->addWidget(likeBar);

        menuVbox->addLayout(likeBarWrapper);
    }

    topLevelButtons = new QHBoxLayout(watchView);
    topLevelButtons->setContentsMargins(0, 3, 0, 0);
    topLevelButtons->setSpacing(0);
    menuVbox->addLayout(topLevelButtons);

    dislikeLabel = new IconLabel("dislike");
    likeLabel = new IconLabel("like", QMargins(0, 0, 15, 0));

    menuWrapper = new QWidget(watchView);
    menuWrapper->setFixedWidth(playerSize.width());
    menuWrapper->setLayout(menuVbox);
    frame->layout()->addWidget(menuWrapper);
}

void WatchView_Ui::setupPlayer(QWidget* watchView, const QSize& playerSize)
{
#ifdef USEMPV
    media = new MediaMPV(watchView);
    media->init();
    media->setVolume(SettingsStore::instance().preferredVolume);
    media->videoWidget()->setFixedSize(playerSize);
    frame->layout()->addWidget(media->videoWidget());

    connect(media, &Media::error, this, [this](const QString& message) { QMessageBox::warning(this, "Media error", message); });
    connect(media, &Media::stateChanged, this, &WatchView_Ui::mediaStateChanged);
    connect(media, &Media::volumeChanged, this, &WatchView_Ui::volumeChanged);
#else
    wePlayer = new WebEnginePlayer(watchView);
    wePlayer->setFixedSize(playerSize);
    frame->layout()->addWidget(wePlayer);
#endif
}

void WatchView_Ui::setupPrimaryInfo(QWidget* watchView, const QSize& playerSize)
{
    primaryInfoHbox = new QHBoxLayout(watchView);
    primaryInfoHbox->setContentsMargins(0, 0, 0, 0);

    channelIcon = new TubeLabel(watchView);
    channelIcon->setClickable(true, false);
    channelIcon->setMaximumSize(55, 48);
    channelIcon->setMinimumSize(55, 48);
    primaryInfoHbox->addWidget(channelIcon);

    primaryInfoVbox = new QVBoxLayout(watchView);

    channelLabel = new ChannelLabel(watchView);
    primaryInfoVbox->addWidget(channelLabel);

    subscribeWidget = new SubscribeWidget(watchView);
    subscribeWidget->layout()->addStretch();
    subscribeWidget->subscribersCountLabel()->setVisible(false);
    primaryInfoVbox->addWidget(subscribeWidget);

    primaryInfoHbox->addLayout(primaryInfoVbox);
    primaryInfoHbox->addStretch();

    primaryInfoWrapper = new QWidget(watchView);
    primaryInfoWrapper->setFixedWidth(playerSize.width());
    primaryInfoWrapper->setLayout(primaryInfoHbox);
    frame->layout()->addWidget(primaryInfoWrapper);
}

void WatchView_Ui::setupTitle(QWidget* watchView, const QSize& playerSize)
{
    titleLabel = new TubeLabel(watchView);
    titleLabel->setFixedWidth(playerSize.width());
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 4));
    titleLabel->setWordWrap(true);
    frame->layout()->addWidget(titleLabel);
}

QSize WatchView_Ui::calcPlayerSize(const QSize& maxSize) const
{
    int playerWidth = maxSize.width();
    int playerHeight = playerWidth * 9/16;

    if (playerHeight > maxSize.height() - 150)
    {
        playerHeight = maxSize.height() - 150;
        playerWidth = playerHeight * 16/9;
    }

    return QSize(playerWidth, playerHeight);
}

void WatchView_Ui::toggleShowMore()
{
    if (showMoreLabel->text() == "SHOW MORE")
    {
        description->setMaximumHeight(QWIDGETSIZE_MAX);
        showMoreLabel->setText("SHOW LESS");
    }
    else
    {
        UIUtilities::setMaximumLines(description, 3);
        showMoreLabel->setText("SHOW MORE");
    }
}

#ifdef USEMPV
void WatchView_Ui::mediaStateChanged(Media::State state)
{
    if (state == Media::ErrorState)
        QMessageBox::critical(this, "Media error", media->errorString());
}

void WatchView_Ui::volumeChanged(double volume)
{
    Q_UNUSED(volume);
    if (media->volumeMuted())
        media->setVolumeMuted(false);
}
#endif
