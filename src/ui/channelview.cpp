#include "channelview.h"
#include "mainwindow.h"
#include "uiutilities.h"

ChannelView* ChannelView::instance()
{
    static ChannelView* wv = new ChannelView;
    return wv;
}

void ChannelView::goBack()
{
    MainWindow::instance()->topbar->alwaysShow = true;
    disconnect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &ChannelView::goBack);
    UIUtilities::clearLayout(pageLayout);
    pageLayout->deleteLater();
    stackedWidget->setCurrentIndex(0);
}

void ChannelView::loadChannel()
{
    stackedWidget->setCurrentIndex(2);

    pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    channelBanner = new QLabel(this);
    channelBanner->setFixedHeight(129);
    channelBanner->setText("sneed");
    pageLayout->addWidget(channelBanner);

    channelHeader = new QHBoxLayout(this);
    channelHeader->setSpacing(10);

    channelIcon = new QLabel(this);
    channelIcon->setFixedSize(55, 48);
    channelIcon->setText("sneed");
    channelHeader->addWidget(channelIcon);

    channelName = new QLabel(this);
    channelName->setText("sneed");
    channelHeader->addWidget(channelName);

    subscriberCount = new QLabel(this);
    subscriberCount->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    subscriberCount->setMargin(25);
    subscriberCount->setText("sneed");
    channelHeader->addWidget(subscriberCount);

    pageLayout->addLayout(channelHeader);

    channelTabs = new QTabWidget(this);
    pageLayout->addWidget(channelTabs);

    MainWindow::instance()->topbar->setVisible(false);
    MainWindow::instance()->topbar->alwaysShow = false;
    connect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &ChannelView::goBack);
}
