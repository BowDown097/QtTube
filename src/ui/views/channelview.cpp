#include "channelview.h"
#include "http.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include <QApplication>

ChannelView* ChannelView::instance()
{
    if (!m_channelView)
        m_channelView = new ChannelView;
    return m_channelView;
}

void ChannelView::goBack()
{
    MainWindow::topbar()->alwaysShow = true;
    MainWindow::topbar()->updatePalette(qApp->palette().alternateBase().color());
    MainWindow::centralWidget()->setCurrentIndex(0);
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &ChannelView::goBack);

    UIUtilities::clearLayout(pageLayout);
    pageLayout->deleteLater();
}

void ChannelView::loadChannel(const QString& channelId)
{
    auto channelResp = InnerTube::instance().getBlocking<InnertubeEndpoints::BrowseChannel>(channelId).response;
    InnertubeObjects::ChannelHeader header = channelResp.header[0];

    MainWindow::centralWidget()->setCurrentIndex(2);

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
    channelName->setText(header.title);
    metaVbox->addWidget(channelName);

    handleAndVideos = new TubeLabel(this);
    handleAndVideos->setText(header.channelHandleText.text + " • " + header.videosCountText.text);
    handleAndVideos->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() - 2));
    metaVbox->addWidget(handleAndVideos);

    metaHbox->addLayout(metaVbox);
    channelHeader->addLayout(metaHbox);

    subscribeHbox = new QHBoxLayout(this);
    subscribeHbox->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    subscribeHbox->setContentsMargins(0, 0, 0, 0);
    subscribeHbox->setSpacing(0);

    subscribeWidget = new SubscribeWidget(this);
    subscribeWidget->setSubscribeButton(header.subscribeButton);
    subscribeHbox->addWidget(subscribeWidget);

    subscribersLabel = new TubeLabel(this);
    subscribersLabel->setStyleSheet(R"(
    border: 1px solid #333;
    font-size: 11px;
    line-height: 24px;
    padding: 0 6px 0 4.5px;
    border-radius: 2px;
    text-align: center;
    )");
    setSubscriberCount(channelResp);
    subscribeHbox->addWidget(subscribersLabel);

    channelHeader->addLayout(subscribeHbox);

    pageLayout->addWidget(channelHeaderWidget);

    channelTabs = new QTabWidget(this);
    pageLayout->addWidget(channelTabs);

    MainWindow::topbar()->alwaysShow = false;
    MainWindow::topbar()->setVisible(false);
    connect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &ChannelView::goBack);

    if (!header.banner.isEmpty())
    {
        HttpReply* bannerReply = Http::instance().get(header.banner[0].url);
        connect(bannerReply, &HttpReply::finished, this, &ChannelView::setBanner);
    }

    HttpReply* iconReply = Http::instance().get(header.avatar[0].url);
    connect(iconReply, &HttpReply::finished, this, &ChannelView::setIcon);
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
    QList<int> red, green, blue;
    red.reserve(256);
    green.reserve(256);
    blue.reserve(256);

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
        QPalette domPal(QColor(std::get<0>(domRgb), std::get<1>(domRgb), std::get<2>(domRgb)));
        channelHeaderWidget->setPalette(domPal);
        MainWindow::topbar()->updatePalette(domPal);
    }
}

void ChannelView::setIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ChannelView::setSubscriberCount(const InnertubeEndpoints::ChannelResponse& channelResp)
{
    QString subscriberCountText = channelResp.header[0].subscriberCountText.text;
    if (!SettingsStore::instance().fullSubs)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        subscribersLabel->setText(subscriberCountText.first(subscriberCountText.lastIndexOf(" ")));
#else
        subscribersLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
#endif
        subscribersLabel->adjustSize();
        return;
    }

    Http http;
    http.setReadTimeout(2000);
    http.setMaxRetries(5);

    // have to catch errors here because this API really, REALLY likes to stop working
    HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelResp.header[0].channelId));
    connect(reply, &HttpReply::error, this, [this, subscriberCountText] {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        subscribersLabel->setText(subscriberCountText.first(subscriberCountText.lastIndexOf(" ")));
#else
        subscribersLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
#endif
        subscribersLabel->adjustSize();
    });
    connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
        int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
        subscribersLabel->setText(QLocale::system().toString(subs));
        subscribersLabel->adjustSize();
    });
}
