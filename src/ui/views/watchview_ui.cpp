#include "watchview_ui.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include <QApplication>

void WatchView_Ui::setupUi(QWidget* watchView)
{
    setupFrame(watchView);
    setupPlayer(watchView);
    setupTitle(watchView);
    setupPrimaryInfo(watchView);
    setupMenu(watchView);
    setupDate(watchView);
    setupDescription(watchView);
    qobject_cast<QVBoxLayout*>(frame->layout())->addStretch(); // disable the layout from stretching on resize
}

void WatchView_Ui::setupDate(QWidget* watchView)
{
    infoSpacer = new QSpacerItem(20, 20);
    frame->layout()->addItem(infoSpacer);

    date = new TubeLabel(watchView);
    date->setFont(QFont(qApp->font().toString(), -1, QFont::DemiBold));
    frame->layout()->addWidget(date);
}

void WatchView_Ui::setupDescription(QWidget* watchView)
{
    description = new TubeLabel(watchView);
    description->setFixedWidth(player->size().width());
    description->setTextFormat(Qt::RichText);
    description->setWordWrap(true);
    UIUtilities::setMaximumLines(description, 3);
    frame->layout()->addWidget(description);

    showMoreLabel = new TubeLabel(watchView);
    showMoreLabel->setAlignment(Qt::AlignCenter);
    showMoreLabel->setClickable(true, false);
    showMoreLabel->setFixedWidth(player->size().width());
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

void WatchView_Ui::setupMenu(QWidget* watchView)
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

    menuWrapper = new QWidget(watchView);
    menuWrapper->setFixedWidth(player->size().width());
    menuWrapper->setLayout(menuVbox);
    frame->layout()->addWidget(menuWrapper);
}

void WatchView_Ui::setupPlayer(QWidget* watchView)
{
    player = new WatchViewPlayer(watchView, MainWindow::size());
    frame->layout()->addWidget(player->widget());
}

void WatchView_Ui::setupPrimaryInfo(QWidget* watchView)
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
    channelLabel->text->setFont(QFont(qApp->font().toString(), -1, QFont::DemiBold));
    primaryInfoVbox->addWidget(channelLabel);

    subscribeWidget = new SubscribeWidget(watchView);
    subscribeWidget->layout->addStretch();
    subscribeWidget->subscribersCountLabel->setVisible(false);
    primaryInfoVbox->addWidget(subscribeWidget);

    primaryInfoHbox->addLayout(primaryInfoVbox);
    primaryInfoHbox->addStretch();

    primaryInfoWrapper = new QWidget(watchView);
    primaryInfoWrapper->setFixedWidth(player->size().width());
    primaryInfoWrapper->setLayout(primaryInfoHbox);
    frame->layout()->addWidget(primaryInfoWrapper);
}

void WatchView_Ui::setupTitle(QWidget* watchView)
{
    titleLabel = new TubeLabel(watchView);
    titleLabel->setFixedWidth(player->size().width());
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 4));
    titleLabel->setWordWrap(true);
    frame->layout()->addWidget(titleLabel);
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
