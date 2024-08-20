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

    handleAndVideos->setFont(QFont(font().toString(), font().pointSize() - 2));
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

    if (auto c4 = std::get_if<InnertubeObjects::ChannelC4Header>(&response.header))
        prepareHeader(*c4);
    else if (auto page = std::get_if<InnertubeObjects::ChannelPageHeader>(&response.header))
        prepareHeader(*page, response.mutations);

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

        ContinuableListWidget* list = new ContinuableListWidget(tab);
        list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        list->verticalScrollBar()->setSingleStep(25);
        grid->addWidget(list, 0, 0, 1, 1);

        connect(list, &ContinuableListWidget::continuationReady, this, [this, list] {
            BrowseHelper::instance()->continuation<InnertubeEndpoints::BrowseChannel>(list, this->channelId);
        });

        channelTabs->addTab(tab, v["tabRenderer"]["title"].toString());
    }
}

void ChannelView::loadTab(const InnertubeEndpoints::ChannelResponse& response, int index)
{
    const QList<ContinuableListWidget*> tabs = channelTabs->findChildren<ContinuableListWidget*>();
    for (ContinuableListWidget* l : tabs)
        l->clear();

    ContinuableListWidget* list = channelTabs->widget(index)->findChild<ContinuableListWidget*>();
    BrowseHelper::instance()->browseChannel(list, index, response);
}

void ChannelView::prepareAvatarAndBanner(const InnertubeObjects::ResponsiveImage& avatar,
                                         const InnertubeObjects::ResponsiveImage& banner)
{
    channelBanner->setFixedHeight(banner.isEmpty() ? 40 : 129);
    MainWindow::topbar()->setAlwaysShow(banner.isEmpty());
    MainWindow::topbar()->setVisible(banner.isEmpty());

    if (auto recAvatar = avatar.recommendedQuality(QSize(48, 48)); recAvatar.has_value())
    {
        HttpReply* iconReply = Http::instance().get(recAvatar->get().url);
        connect(iconReply, &HttpReply::finished, this, &ChannelView::setIcon);
    }

    if (auto bestBanner = banner.bestQuality(); bestBanner.has_value())
    {
        HttpReply* bannerReply = Http::instance().get(bestBanner->get().url);
        connect(bannerReply, &HttpReply::finished, this, &ChannelView::setBanner);
    }
}

void ChannelView::prepareHeader(const InnertubeObjects::ChannelC4Header& c4Header)
{
    channelName->setText(c4Header.title);
    handleAndVideos->setText(c4Header.channelHandleText.text + " • " + c4Header.videosCountText.text);
    subscribeWidget->setSubscribeButton(c4Header.subscribeButton);
    subscribeWidget->setSubscriberCount(c4Header.subscriberCountText.text, c4Header.channelId);

    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow()))
        mainWindow->setWindowTitle(c4Header.title + " - " + QTTUBE_APP_NAME);

    prepareAvatarAndBanner(c4Header.avatar, c4Header.banner);
}

void ChannelView::prepareHeader(const InnertubeObjects::ChannelPageHeader& pageHeader,
                                const QList<InnertubeObjects::EntityMutation>& mutations)
{
    QString channelHandle = pageHeader.metadata.metadataRows.value(0).value(0);
    QString subCount = pageHeader.metadata.metadataRows.value(1).value(0);
    QString videosCount = pageHeader.metadata.metadataRows.value(1).value(1);

    channelName->setText(pageHeader.title.text.content);
    handleAndVideos->setText(channelHandle + ' ' + pageHeader.metadata.delimiter + ' ' + videosCount);
    subscribeWidget->setSubscriberCount(subCount, channelId);

    if (pageHeader.subscribeButton)
        subscribeWidget->setSubscribeButton(pageHeader.subscribeButton.value(), pageHeader.subscribeButton->isSubscribed(mutations));

    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow()))
        mainWindow->setWindowTitle(pageHeader.title.text.content + " - " + QTTUBE_APP_NAME);

    prepareAvatarAndBanner(pageHeader.avatar, pageHeader.banner);
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
