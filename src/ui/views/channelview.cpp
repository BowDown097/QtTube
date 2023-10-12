#include "channelview.h"
#include "http.h"
#include "ui/browsehelper.h"
#include "ui/forms/mainwindow.h"
#include <QApplication>
#include <QScrollBar>

ChannelView::~ChannelView()
{
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, nullptr);
}

ChannelView::ChannelView(const QString& channelId)
    : channelBanner(new QLabel(this)),
      channelHeaderWidget(new QWidget(this)),
      channelIcon(new QLabel(this)),
      channelName(new TubeLabel(this)),
      channelTabs(new QTabWidget(this)),
      handleAndVideos(new TubeLabel(this)),
      metaHbox(new QHBoxLayout),
      metaVbox(new QVBoxLayout),
      pageLayout(new QVBoxLayout(this)),
      subscribeWidget(new SubscribeWidget(this))
{
    auto channelResp = InnerTube::instance().getBlocking<InnertubeEndpoints::BrowseChannel>(channelId).response;
    this->channelId = channelId;

    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    channelBanner->setFixedHeight(129);
    pageLayout->addWidget(channelBanner);

    channelHeaderWidget->setFixedHeight(63);
    channelHeaderWidget->setAutoFillBackground(true);

    channelHeader = new QHBoxLayout(channelHeaderWidget);
    channelHeader->setSpacing(10);

    metaHbox->setContentsMargins(0,0,0,0);

    channelIcon->setFixedSize(55, 48);
    metaHbox->addWidget(channelIcon);

    channelName->setText(channelResp.header.title);
    metaVbox->addWidget(channelName);

    handleAndVideos->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() - 2));
    handleAndVideos->setText(channelResp.header.channelHandleText.text + " • " + channelResp.header.videosCountText.text);
    metaVbox->addWidget(handleAndVideos);

    metaHbox->addLayout(metaVbox);
    channelHeader->addLayout(metaHbox);

    subscribeWidget->layout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    subscribeWidget->setSubscribeButton(channelResp.header.subscribeButton);
    subscribeWidget->setSubscriberCount(channelResp.header.subscriberCountText.text, channelResp.header.channelId);
    channelHeader->addWidget(subscribeWidget);

    pageLayout->addWidget(channelHeaderWidget);

    channelTabs->tabBar()->setAutoFillBackground(true);
    channelTabs->tabBar()->setDocumentMode(true);
    channelTabs->tabBar()->setExpanding(false);
    pageLayout->addWidget(channelTabs);
    setTabsAndStyles(channelResp);

    MainWindow::topbar()->alwaysShow = false;
    MainWindow::topbar()->setVisible(false);
}

void ChannelView::hotLoadChannel(const QString& channelId)
{
    if (this->channelId == channelId)
        return;

    auto channelResp = InnerTube::instance().getBlocking<InnertubeEndpoints::BrowseChannel>(channelId).response;
    this->channelId = channelId;

    channelName->setText(channelResp.header.title);
    handleAndVideos->setText(channelResp.header.channelHandleText.text + " • " + channelResp.header.videosCountText.text);
    subscribeWidget->setSubscribeButton(channelResp.header.subscribeButton);
    subscribeWidget->setSubscriberCount(channelResp.header.subscriberCountText.text, channelResp.header.channelId);

    disconnect(channelTabs, &QTabWidget::currentChanged, nullptr, nullptr);
    channelTabs->clear();
    setTabsAndStyles(channelResp);
}

void ChannelView::setBanner(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelBanner->setPixmap(pixmap.scaled(channelBanner->width(), channelBanner->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ChannelView::setIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ChannelView::setTabsAndStyles(const InnertubeEndpoints::ChannelResponse& channelResp)
{
    connect(channelTabs, &QTabWidget::currentChanged, this, [this, channelResp](int index) {
        for (QListWidget* l : channelTabs->findChildren<QListWidget*>())
            l->clear();
        QListWidget* list = channelTabs->widget(index)->findChild<QListWidget*>();
        BrowseHelper::instance()->browseChannel(list, index, channelResp);
    });

    const QJsonArray tabs = channelResp.contents["twoColumnBrowseResultsRenderer"]["tabs"].toArray();
    for (const QJsonValue& v : tabs)
    {
        if (!v["tabRenderer"].isObject())
            continue;

        QWidget* tab = new QWidget;
        QGridLayout* grid = new QGridLayout(tab);
        grid->setContentsMargins(0, 0, 0, 0);
        QListWidget* list = new QListWidget(tab);
        list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        list->verticalScrollBar()->setSingleStep(25);
        grid->addWidget(list, 0, 0, 1, 1);
        channelTabs->addTab(tab, v["tabRenderer"]["title"].toString());
    }

    if (!channelResp.header.banners.isEmpty())
    {
        HttpReply* bannerReply = Http::instance().get(channelResp.header.banners[0].url);
        connect(bannerReply, &HttpReply::finished, this, &ChannelView::setBanner);
    }

    HttpReply* iconReply = Http::instance().get(channelResp.header.avatars[0].url);
    connect(iconReply, &HttpReply::finished, this, &ChannelView::setIcon);
}
