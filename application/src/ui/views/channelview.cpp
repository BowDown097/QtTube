#include "channelview.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/browsehelper.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QScrollBar>

ChannelView::~ChannelView()
{
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, nullptr);
    if (QMainWindow* mainWindow = UIUtils::getMainWindow())
        mainWindow->setWindowTitle(QTTUBE_APP_NAME);
}

ChannelView::ChannelView(const QString& channelId)
    : channelBanner(new TubeLabel(this)),
      channelHeaderContainer(new QWidget(this)),
      channelHeaderLayout(new QHBoxLayout(channelHeaderContainer)),
      channelIcon(new TubeLabel(this)),
      channelName(new TubeLabel(this)),
      channelTabs(new QTabWidget(this)),
      handleAndVideos(new TubeLabel(this)),
      metaHbox(new QHBoxLayout),
      metaVbox(new QVBoxLayout),
      pageLayout(new QVBoxLayout(this)),
      subscribeWidget(new SubscribeWidget(this))
{
    metaHbox->setContentsMargins(0, 0, 0, 0);

    pageLayout->setSpacing(0);

    channelBanner->setMinimumSize(1, 1);
    channelBanner->setScaledContents(true);
    pageLayout->addWidget(channelBanner);

    if (qtTubeApp->settings().autoHideTopBar)
        pageLayout->setContentsMargins(0, 0, 0, 0);
    else
        pageLayout->setContentsMargins(0, MainWindow::topbar()->height(), 0, 0);

    channelHeaderContainer->setFixedHeight(63);
    channelHeaderContainer->setAutoFillBackground(true);
    channelHeaderLayout->setSpacing(10);

    channelIcon->setFixedSize(48, 48);
    channelIcon->setScaledContents(true);
    metaHbox->addWidget(channelIcon);
    metaHbox->addSpacerItem(new QSpacerItem(7, 0));

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
    if (this->channelId != channelId)
    {
        disconnect(channelTabs, &QTabWidget::currentChanged, nullptr, nullptr);
        channelTabs->clear();
        loadChannel(channelId);
    }
}

void ChannelView::loadChannel(const QString& channelId)
{
    this->channelId = channelId;
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::ChannelReply* reply = plugin->interface->getChannel(channelId, {}, {});

        QEventLoop loop;
        connect(reply, &QtTube::ChannelReply::exception, this, [this, &loop](const QtTube::PluginException& ex) {
            loop.quit();
            emit loadFailed(ex);
        });
        connect(reply, &QtTube::ChannelReply::finished, this, [this, &loop](const QtTube::ChannelData& data) {
            loop.quit();
            processData(data);
        });
        loop.exec();
    }
}

void ChannelView::loadTab(std::any requestData, int index)
{
    const QList<ContinuableListWidget*> tabWidgets = channelTabs->findChildren<ContinuableListWidget*>();
    for (ContinuableListWidget* list : tabWidgets)
        list->clear();

    ContinuableListWidget* list = channelTabs->widget(index)->findChild<ContinuableListWidget*>();
    BrowseHelper::instance()->browseChannel(list, index, channelId, requestData);
}

void ChannelView::processData(const QtTube::ChannelData& data)
{
    processHeader(data.header.value());
    processTabs(data.tabs);
    connect(channelTabs, &QTabWidget::currentChanged, this, [this, tabs = data.tabs](int index) {
        loadTab(tabs.at(index).requestData, index);
    });
}

void ChannelView::processHeader(const QtTube::ChannelHeader& header)
{
    channelName->setText(header.channelText);
    handleAndVideos->setText(header.channelSubtext);
    subscribeWidget->setData(header.subscribeButton);

    if (QMainWindow* mainWindow = UIUtils::getMainWindow())
        mainWindow->setWindowTitle(header.channelText + " - " + QTTUBE_APP_NAME);

    bool hasAvatar = !header.avatarUrl.isEmpty();
    bool hasBanner = !header.bannerUrl.isEmpty();

    if (hasBanner)
        channelBanner->setFixedHeight(129);
    else if (qtTubeApp->settings().autoHideTopBar)
        channelBanner->setFixedHeight(MainWindow::topbar()->height() + 20);

    if (qtTubeApp->settings().autoHideTopBar)
    {
        MainWindow::topbar()->setAlwaysShow(!hasBanner);
        MainWindow::topbar()->setVisible(!hasBanner);
    }

    if (hasAvatar)
        channelIcon->setImage(header.avatarUrl);
    if (hasBanner)
        channelBanner->setImage(header.bannerUrl);
}

void ChannelView::processTabs(const QList<QtTube::ChannelTabData>& tabs)
{
    if (tabs.isEmpty())
        return;

    ContinuableListWidget* firstListWidget{};

    for (qsizetype i = 0; i < tabs.size(); ++i)
    {
        QWidget* tabWidget = new QWidget;

        QGridLayout* grid = new QGridLayout(tabWidget);
        grid->setContentsMargins(0, 0, 0, 0);

        ContinuableListWidget* list = new ContinuableListWidget(tabWidget);
        list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        list->verticalScrollBar()->setSingleStep(25);
        grid->addWidget(list, 0, 0, 1, 1);

        if (!firstListWidget)
            firstListWidget = list;

        connect(list, &ContinuableListWidget::continuationReady, this, [this, data = tabs[i].requestData, i, list] {
            if (list->continuationData.has_value())
                BrowseHelper::instance()->browseChannel(list, i, channelId, data);
        });

        channelTabs->addTab(tabWidget, tabs[i].title);
    }

    BrowseHelper::instance()->processChannelTabItems(firstListWidget, tabs.front().items);
}
