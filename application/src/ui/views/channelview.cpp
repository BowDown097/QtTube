#include "channelview.hpp"
#include "mainwindow.hpp"
#include "qttubeapplication.hpp"
#include "ui/widgets/subscribe/subscribewidget.hpp"
#include "ui/widgets/tabwidgets/channeltabwidget.hpp"
#include "ui/widgets/topbar/topbar.hpp"
#include "utils/uiutils.hpp"
#include <QBoxLayout>
#include <qttube-plugin/providers/providertypes.h>

ChannelView::~ChannelView()
{
    if (MainWindow* mainWindow = UIUtils::getMainWindow(false))
    {
        mainWindow->setWindowTitle(QTTUBE_APP_NAME);
        disconnect(mainWindow->topbar()->logo, &TubeLabel::clicked, this, nullptr);
    }
}

ChannelView::ChannelView(const QString& channelId, PluginEntry* plugin)
    : m_avatar(new TubeLabel(this)),
      m_banner(new TubeLabel(this)),
      m_headerContainer(new QWidget(this)),
      m_headerLayout(new QHBoxLayout(m_headerContainer)),
      m_subtextLabel(new TubeLabel(this)),
      m_metaHbox(new QHBoxLayout),
      m_metaVbox(new QVBoxLayout),
      m_nameLabel(new TubeLabel(this)),
      m_plugin(plugin),
      m_pageLayout(new QVBoxLayout(this)),
      m_subscribeWidget(new SubscribeWidget(plugin, this)),
      m_tabWidget(new ChannelTabWidget(this))
{
    m_metaHbox->setContentsMargins(0, 0, 0, 0);

    if (qtTubeApp->settings().autoHideTopBar)
        m_pageLayout->setContentsMargins(0, 0, 0, 0);
    else
        m_pageLayout->setContentsMargins(0, UIUtils::getMainWindow()->topbar()->height(), 0, 0);

    m_banner->setMinimumSize(1, 1);
    m_banner->setScaledContents(true);
    m_pageLayout->addWidget(m_banner);

    m_headerContainer->setFixedHeight(63);
    m_headerContainer->setAutoFillBackground(true);
    m_headerLayout->setSpacing(10);

    m_avatar->setFixedSize(48, 48);
    m_avatar->setScaledContents(true);
    m_metaHbox->addWidget(m_avatar);
    m_metaHbox->addSpacerItem(new QSpacerItem(7, 0));

    m_subtextLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    m_metaVbox->addWidget(m_nameLabel);
    m_metaVbox->addWidget(m_subtextLabel);
    m_metaHbox->addLayout(m_metaVbox);

    m_headerLayout->addLayout(m_metaHbox);
    m_headerLayout->addWidget(m_subscribeWidget, 0, Qt::AlignRight | Qt::AlignVCenter);

    m_pageLayout->addWidget(m_headerContainer);
    m_pageLayout->addWidget(m_tabWidget);

    addAction(m_tabWidget->makeReloadAction());
    loadChannel(channelId);
}

void ChannelView::hotLoadChannel(const QString& channelId)
{
    if (m_channelId != channelId)
    {
        m_tabWidget->clear();
        loadChannel(channelId);
    }
}

void ChannelView::loadChannel(const QString& channelId)
{
    assert(m_plugin->providers.channel != nullptr);
    m_channelId = channelId;

    QtTubePlugin::ChannelReply* reply = m_plugin->providers.channel->getChannel(channelId, {}, {});
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

void ChannelView::processData(const QtTubePlugin::ChannelData& data)
{
    m_nameLabel->setText(data.header->channelText);
    m_subtextLabel->setText(data.header->channelSubtext);
    m_subscribeWidget->setData(m_channelId, data.header->subscribeButton);

    MainWindow* mainWindow = UIUtils::getMainWindow();
    mainWindow->setWindowTitle(data.header->channelText % " - " % QTTUBE_APP_NAME);

    bool hasAvatar = !data.header->avatarUrl.isEmpty();
    bool hasBanner = !data.header->bannerUrl.isEmpty();

    if (hasBanner)
        m_banner->setFixedHeight(129);
    else if (qtTubeApp->settings().autoHideTopBar)
        m_banner->setFixedHeight(mainWindow->topbar()->height() + 20);

    if (qtTubeApp->settings().autoHideTopBar)
    {
        mainWindow->topbar()->setAlwaysShow(!hasBanner);
        mainWindow->topbar()->setVisible(!hasBanner);
    }

    if (hasAvatar)
        m_avatar->setImage(data.header->avatarUrl);
    else
        m_avatar->setPixmap(QPixmap());

    if (hasBanner)
        m_banner->setImage(data.header->bannerUrl);
    else
        m_banner->setPixmap(QPixmap());

    m_tabWidget->setData(m_plugin, m_channelId, data.tabs);
}