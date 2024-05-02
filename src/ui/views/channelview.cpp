#include "channelview.h"
#include "http.h"
#include "mainwindow.h"
#include "ui/browsehelper.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include <QApplication>
#include <QBoxLayout>
#include <QScrollBar>

ChannelView::~ChannelView()
{
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, nullptr);
    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow()))
        mainWindow->setWindowTitle(QTTUBE_APP_NAME);
}

ChannelView::ChannelView(const QString& channelId)
    : channelBanner(new QLabel(this)),
      channelHeaderContainer(new QWidget(this)),
      channelHeaderLayout(new QHBoxLayout(channelHeaderContainer)),
      channelIcon(new QLabel(this)),
      channelName(new TubeLabel(this)),
      channelTabs(new QTabWidget(this)),
      handleAndVideos(new TubeLabel(this)),
      metaHbox(new QHBoxLayout),
      metaVbox(new QVBoxLayout),
      pageLayout(new QVBoxLayout(this)),
      subscribeWidget(new SubscribeWidget(this))
{
    metaHbox->setContentsMargins(0, 0, 0, 0);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);
    pageLayout->addWidget(channelBanner);

    channelHeaderContainer->setFixedHeight(63);
    channelHeaderContainer->setAutoFillBackground(true);
    channelHeaderLayout->setSpacing(10);

    channelIcon->setFixedSize(55, 48);
    metaHbox->addWidget(channelIcon);

    metaVbox->addWidget(channelName);

    handleAndVideos->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() - 2));
    metaVbox->addWidget(handleAndVideos);

    metaHbox->addLayout(metaVbox);
    channelHeaderLayout->addLayout(metaHbox);

    subscribeWidget->layout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    channelHeaderLayout->addWidget(subscribeWidget);

    pageLayout->addWidget(channelHeaderContainer);

    channelTabs->tabBar()->setAutoFillBackground(true);
    channelTabs->tabBar()->setDocumentMode(true);
    channelTabs->tabBar()->setExpanding(false);
    pageLayout->addWidget(channelTabs);

    loadChannel(channelId);
}

void ChannelView::hotLoadChannel(const QString& channelId)
{
    if (this->channelId == channelId)
        return;

    disconnect(channelTabs, &QTabWidget::currentChanged, nullptr, nullptr);
    channelTabs->clear();
    loadChannel(channelId);
}

void ChannelView::loadChannel(const QString& channelId)
{
    this->channelId = channelId;
    auto response = InnerTube::instance()->getBlocking<InnertubeEndpoints::BrowseChannel>(channelId).response;

    QString channelHandle = response.header.metadata.metadataRows.value(0).value(0);
    QString subCount = response.header.metadata.metadataRows.value(1).value(0);
    QString videosCount = response.header.metadata.metadataRows.value(1).value(1);

    channelName->setText(response.header.title.text.content);
    handleAndVideos->setText(channelHandle + ' ' + response.header.metadata.delimiter + ' ' + videosCount);
    subscribeWidget->setSubscriberCount(subCount, channelId);

    if (response.header.subscribeButton)
    {
        subscribeWidget->setSubscribeButton(response.header.subscribeButton.value(),
            response.header.subscribeButton->isSubscribed(response.mutations));
    }

    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow()))
        mainWindow->setWindowTitle(response.header.title.text.content + " - " + QTTUBE_APP_NAME);

    connect(channelTabs, &QTabWidget::currentChanged, this,
            std::bind(&ChannelView::loadTab, this, response, std::placeholders::_1));

    const QJsonArray tabs = response.contents["twoColumnBrowseResultsRenderer"]["tabs"].toArray();
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

    bool hasBanner = !response.header.banner.isEmpty();
    channelBanner->setFixedHeight(hasBanner ? 129 : 40);
    MainWindow::topbar()->setAlwaysShow(!hasBanner);
    MainWindow::topbar()->setVisible(!hasBanner);

    if (hasBanner)
    {
        HttpReply* bannerReply = Http::instance().get(response.header.banner.bestQuality().url);
        connect(bannerReply, &HttpReply::finished, this, &ChannelView::setBanner);
    }

    if (!response.header.avatar.isEmpty())
    {
        HttpReply* iconReply = Http::instance().get(response.header.avatar.recommendedQuality(QSize(48, 48)).url);
        connect(iconReply, &HttpReply::finished, this, &ChannelView::setIcon);
    }
}

void ChannelView::loadTab(const InnertubeEndpoints::ChannelResponse& response, int index)
{
    for (QListWidget* l : channelTabs->findChildren<QListWidget*>())
        l->clear();

    QListWidget* list = channelTabs->widget(index)->findChild<QListWidget*>();
    BrowseHelper::instance()->browseChannel(list, index, response);
}

void ChannelView::setBanner(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelBanner->setPixmap(pixmap.scaled(channelBanner->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ChannelView::setIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
