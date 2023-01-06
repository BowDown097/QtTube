#include "channelview.h"
#include "http.h"
#include "innertube.h"
#include "mainwindow.h"
#include "../settingsstore.h"
#include "uiutilities.h"
#include <QApplication>

ChannelView* ChannelView::instance()
{
    static ChannelView* wv = new ChannelView;
    return wv;
}

void ChannelView::goBack()
{
    MainWindow::instance()->topbar->alwaysShow = true;
    MainWindow::instance()->topbar->updateColors(QApplication::palette().color(QPalette::AlternateBase));
    disconnect(MainWindow::instance()->topbar->logo, &TubeLabel::clicked, this, &ChannelView::goBack);
    UIUtilities::clearLayout(pageLayout);
    pageLayout->deleteLater();
    stackedWidget->setCurrentIndex(0);
}

void ChannelView::loadChannel(const QString& channelId)
{
    InnertubeEndpoints::ChannelResponse channelResp = InnerTube::instance().get<InnertubeEndpoints::BrowseChannel>(channelId).response;
    stackedWidget->setCurrentIndex(2);

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

    channelName = new TubeLabel(this);
    channelName->setText(channelResp.header[0].title);
    metaVbox->addWidget(channelName);

    handleAndVideos = new TubeLabel(this);
    handleAndVideos->setText(channelResp.header[0].channelHandleText.text + " • " + channelResp.header[0].videosCountText.text);
    handleAndVideos->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() - 2));
    metaVbox->addWidget(handleAndVideos);

    metaHbox->addLayout(metaVbox);
    channelHeader->addLayout(metaHbox);

    subscriberCount = new TubeLabel(this);
    subscriberCount->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    subscriberCount->setText(channelResp.header[0].subscriberCountText.text);
    channelHeader->addWidget(subscriberCount);

    pageLayout->addWidget(channelHeaderWidget);

    channelTabs = new QTabWidget(this);
    pageLayout->addWidget(channelTabs);

    MainWindow::instance()->topbar->alwaysShow = false;
    MainWindow::instance()->topbar->setVisible(false);
    connect(MainWindow::instance()->topbar->logo, &TubeLabel::clicked, this, &ChannelView::goBack);

    if (!channelResp.header[0].banner.isEmpty())
    {
        HttpReply* bannerReply = Http::instance().get(channelResp.header[0].banner[0].url);
        QObject::connect(bannerReply, &HttpReply::finished, this, &ChannelView::setBanner);
    }

    HttpReply* iconReply = Http::instance().get(channelResp.header[0].avatar[0].url);
    QObject::connect(iconReply, &HttpReply::finished, this, &ChannelView::setIcon);
}

// below 2 methods courtesy of https://stackoverflow.com/a/61581999 (with some improvements)
int ChannelView::getDominant(const QList<int>& arr)
{
    int max = arr[0];
    int index = 0;
    for (int i = 0; i < 255; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
            index = i;
        }
    }

    return index;
}

std::tuple<int, int, int> ChannelView::getDominantRgb(const QImage& img)
{
    QRgb* ct;
    QList<int> red(256);
    QList<int> green(256);
    QList<int> blue(256);

    for (int i = 0; i < img.height(); i++)
    {
        ct = (QRgb*)img.scanLine(i);
        for (int j = 0; j < img.width(); j++)
        {
            red[qRed(ct[j])]++;
            green[qGreen(ct[j])]++;
            blue[qBlue(ct[j])]++;
        }
    }

    return std::tuple<int, int, int>(getDominant(red), getDominant(green), getDominant(blue));
}

void ChannelView::setBanner(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelBanner->setPixmap(pixmap.scaled(channelBanner->width(), channelBanner->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    if (SettingsStore::instance().themedChannels)
    {
        std::tuple<int, int, int> domRgb = getDominantRgb(pixmap.toImage());
        QColor domColor(std::get<0>(domRgb), std::get<1>(domRgb), std::get<2>(domRgb));

        QPalette domPal;
        domPal.setColor(QPalette::Window, domColor);

        channelHeaderWidget->setPalette(domPal);
        MainWindow::instance()->topbar->updateColors(domColor);
    }
}

void ChannelView::setIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
