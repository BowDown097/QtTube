#include "channelview.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/browsehelper.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include "ui/widgets/topbar/topbar.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QScrollBar>

ChannelView::~ChannelView()
{
    if (MainWindow* mainWindow = UIUtils::getMainWindow(false))
    {
        mainWindow->setWindowTitle(QTTUBE_APP_NAME);
        disconnect(mainWindow->topbar()->logo, &TubeLabel::clicked, this, nullptr);
    }
}

ChannelView::ChannelView(const QString& channelId, PluginEntry* plugin)
    : m_channelBanner(new TubeLabel(this)),
      m_channelHeaderContainer(new QWidget(this)),
      m_channelHeaderLayout(new QHBoxLayout(m_channelHeaderContainer)),
      m_channelIcon(new TubeLabel(this)),
      m_channelNameLabel(new TubeLabel(this)),
      m_channelTabs(new QTabWidget(this)),
      m_subtextLabel(new TubeLabel(this)),
      m_metaHbox(new QHBoxLayout),
      m_metaVbox(new QVBoxLayout),
      m_plugin(plugin),
      m_pageLayout(new QVBoxLayout(this)),
      m_subscribeWidget(new SubscribeWidget(plugin, this))
{
    m_metaHbox->setContentsMargins(0, 0, 0, 0);

    m_pageLayout->setSpacing(0);

    m_channelBanner->setMinimumSize(1, 1);
    m_channelBanner->setScaledContents(true);
    m_pageLayout->addWidget(m_channelBanner);

    if (qtTubeApp->settings().autoHideTopBar)
        m_pageLayout->setContentsMargins(0, 0, 0, 0);
    else
        m_pageLayout->setContentsMargins(0, UIUtils::getMainWindow()->topbar()->height(), 0, 0);

    m_channelHeaderContainer->setFixedHeight(63);
    m_channelHeaderContainer->setAutoFillBackground(true);
    m_channelHeaderLayout->setSpacing(10);

    m_channelIcon->setFixedSize(48, 48);
    m_channelIcon->setScaledContents(true);
    m_metaHbox->addWidget(m_channelIcon);
    m_metaHbox->addSpacerItem(new QSpacerItem(7, 0));

    m_metaVbox->addWidget(m_channelNameLabel);

    m_subtextLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    m_metaVbox->addWidget(m_subtextLabel);

    m_metaHbox->addLayout(m_metaVbox);
    m_channelHeaderLayout->addLayout(m_metaHbox);

    m_subscribeWidget->layout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelHeaderLayout->addWidget(m_subscribeWidget);

    m_pageLayout->addWidget(m_channelHeaderContainer);

    m_channelTabs->tabBar()->setAutoFillBackground(true);
    m_channelTabs->tabBar()->setDocumentMode(true);
    m_channelTabs->tabBar()->setExpanding(false);
    m_pageLayout->addWidget(m_channelTabs);

    loadChannel(channelId);
}

void ChannelView::hotLoadChannel(const QString& channelId)
{
    if (m_channelId != channelId)
    {
        disconnect(m_channelTabs, &QTabWidget::currentChanged, nullptr, nullptr);
        m_channelTabs->clear();
        loadChannel(channelId);
    }
}

void ChannelView::loadChannel(const QString& channelId)
{
    m_channelId = channelId;
    if (QtTubePlugin::ChannelReply* reply = m_plugin->interface->getChannel(channelId, {}, {}))
    {
        QEventLoop loop;
        connect(reply, &QtTubePlugin::ChannelReply::exception, this, [this, &loop](const QtTubePlugin::Exception& ex) {
            loop.quit();
            emit loadFailed(ex);
        });
        connect(reply, &QtTubePlugin::ChannelReply::finished, this, [this, &loop](const QtTubePlugin::ChannelData& data) {
            loop.quit();
            processData(data);
        });
        loop.exec();
    }
    else
    {
        emit loadFailed(QtTubePlugin::Exception("No method has been provided."));
    }
}

void ChannelView::loadTab(std::any requestData, int index)
{
    const QList<ContinuableListWidget*> tabWidgets = m_channelTabs->findChildren<ContinuableListWidget*>();
    for (ContinuableListWidget* list : tabWidgets)
        list->clear();

    ContinuableListWidget* list = m_channelTabs->widget(index)->findChild<ContinuableListWidget*>();
    BrowseHelper::instance()->browseChannel(list, index, m_channelId, requestData);
}

void ChannelView::processData(const QtTubePlugin::ChannelData& data)
{
    processHeader(data.header.value());
    processTabs(data.tabs);
    connect(m_channelTabs, &QTabWidget::currentChanged, this, [this, tabs = data.tabs](int index) {
        loadTab(tabs.at(index).requestData, index);
    });
}

void ChannelView::processHeader(const QtTubePlugin::ChannelHeader& header)
{
    m_channelNameLabel->setText(header.channelText);
    m_subtextLabel->setText(header.channelSubtext);
    m_subscribeWidget->setData(header.subscribeButton);

    MainWindow* mainWindow = UIUtils::getMainWindow();
    mainWindow->setWindowTitle(header.channelText % " - " % QTTUBE_APP_NAME);

    bool hasAvatar = !header.avatarUrl.isEmpty();
    bool hasBanner = !header.bannerUrl.isEmpty();

    if (hasBanner)
        m_channelBanner->setFixedHeight(129);
    else if (qtTubeApp->settings().autoHideTopBar)
        m_channelBanner->setFixedHeight(mainWindow->topbar()->height() + 20);

    if (qtTubeApp->settings().autoHideTopBar)
    {
        mainWindow->topbar()->setAlwaysShow(!hasBanner);
        mainWindow->topbar()->setVisible(!hasBanner);
    }

    if (hasAvatar)
        m_channelIcon->setImage(header.avatarUrl);
    else
        m_channelIcon->setPixmap(QPixmap());

    if (hasBanner)
        m_channelBanner->setImage(header.bannerUrl);
    else
        m_channelBanner->setPixmap(QPixmap());
}

void ChannelView::processTabs(const QList<QtTubePlugin::ChannelTabData>& tabs)
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
                BrowseHelper::instance()->browseChannel(list, i, m_channelId, data);
        });

        m_channelTabs->addTab(tabWidget, tabs[i].title);
    }

    BrowseHelper::instance()->processChannelTabItems(firstListWidget, m_plugin, tabs.front().items);
}
