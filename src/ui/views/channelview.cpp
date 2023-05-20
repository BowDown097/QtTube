#include "channelview.h"
#include "http.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/browsehelper.h"
#include "ui/forms/mainwindow.h"
#include <QApplication>
#include <QScrollBar>

ChannelView::~ChannelView()
{
    MainWindow::topbar()->updatePalette(qApp->palette().alternateBase().color());
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, nullptr);
}

ChannelView::ChannelView(const QString& channelId)
{
    auto channelResp = InnerTube::instance().getBlocking<InnertubeEndpoints::BrowseChannel>(channelId).response;
    this->channelId = channelId;

    pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    channelBanner = new QLabel(this);
    channelBanner->setFixedHeight(129);
    pageLayout->addWidget(channelBanner);

    channelHeaderWidget = new QWidget(this);
    channelHeaderWidget->setFixedHeight(63);
    channelHeaderWidget->setAutoFillBackground(true);

    channelHeader = new QHBoxLayout(channelHeaderWidget);
    channelHeader->setSpacing(10);

    metaHbox = new QHBoxLayout(this);
    metaHbox->setContentsMargins(0,0,0,0);
    metaVbox = new QVBoxLayout(this);

    channelIcon = new QLabel(this);
    channelIcon->setFixedSize(55, 48);
    metaHbox->addWidget(channelIcon);

    channelName = new TubeLabel(channelResp.header.title, this);
    metaVbox->addWidget(channelName);

    handleAndVideos = new TubeLabel(channelResp.header.channelHandleText.text + " • " + channelResp.header.videosCountText.text, this);
    handleAndVideos->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() - 2));
    metaVbox->addWidget(handleAndVideos);

    metaHbox->addLayout(metaVbox);
    channelHeader->addLayout(metaHbox);

    subscribeWidget = new SubscribeWidget(this);
    subscribeWidget->layout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    subscribeWidget->setSubscribeButton(channelResp.header.subscribeButton);
    subscribeWidget->setSubscriberCount(channelResp.header.subscriberCountText.text, channelResp.header.channelId);
    channelHeader->addWidget(subscribeWidget);

    pageLayout->addWidget(channelHeaderWidget);

    channelTabs = new QTabWidget(this);
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

// below 2 methods courtesy of https://stackoverflow.com/a/61581999 (with some improvements)
int ChannelView::getDominant(int arr[256])
{
    int max = arr[0];
    int index = 0;
    for (int i = 0; i < 256; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
            index = i;
        }
    }

    return index;
}

ChannelView::Rgb ChannelView::getDominantRgb(const QImage& img)
{
    int red[256] = {};
    int green[256] = {};
    int blue[256] = {};

    for (int i = 0; i < img.height(); i++)
    {
        const QRgb* ct = reinterpret_cast<const QRgb*>(img.scanLine(i));
        for (int j = 0; j < img.width(); j++)
        {
            red[qRed(ct[j])]++;
            green[qGreen(ct[j])]++;
            blue[qBlue(ct[j])]++;
        }
    }

    return Rgb{getDominant(red), getDominant(green), getDominant(blue)};
}

void ChannelView::setBanner(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelBanner->setPixmap(pixmap.scaled(channelBanner->width(), channelBanner->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    if (SettingsStore::instance().themedChannels)
    {
        Rgb domRgb = getDominantRgb(pixmap.toImage());
        QPalette domPal(QColor(domRgb.r, domRgb.g, domRgb.b));
        channelHeaderWidget->setPalette(domPal);
        channelTabs->tabBar()->setPalette(domPal);
        subscribeWidget->setPreferredPalette(domPal);
        MainWindow::topbar()->updatePalette(domPal);
    }
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
        if (!v.toObject().contains("tabRenderer"))
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
