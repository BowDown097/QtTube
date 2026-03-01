#include "browsechannelrenderer.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include <QBoxLayout>
#include <QMessageBox>

BrowseChannelRenderer::BrowseChannelRenderer(PluginEntry* plugin, QWidget* parent)
    : QWidget(parent),
      m_descriptionLabel(new TubeLabel(this)),
      m_layout(new QHBoxLayout(this)),
      m_metadataLabel(new TubeLabel(this)),
      m_subscribeWidget(new SubscribeWidget(plugin, this)),
      m_textLayout(new QVBoxLayout),
      m_thumbLabel(new TubeLabel(this)),
      m_titleLabel(new ChannelLabel(plugin, this))
{
    m_titleLabel->text->setFont(QFont(font().toString(), font().pointSize() + 2, QFont::Bold));
    m_titleLabel->addStretch();
    m_textLayout->addWidget(m_titleLabel);

    m_textLayout->addWidget(m_metadataLabel);

    m_descriptionLabel->setMaximumLines(2);
    m_descriptionLabel->setWordWrap(true);
    m_textLayout->addWidget(m_descriptionLabel);

    m_subscribeWidget->layout->addStretch();
    m_textLayout->addWidget(m_subscribeWidget);

    m_thumbLabel->setClickable(true);
    m_thumbLabel->setFixedSize(80, 80);
    m_thumbLabel->setScaledContents(true);
    m_layout->addWidget(m_thumbLabel);

    m_layout->addLayout(m_textLayout);

    connect(m_thumbLabel, &TubeLabel::clicked, this, [this, plugin] { ViewController::loadChannel(m_channelId, plugin); });
}

void BrowseChannelRenderer::setData(const QtTubePlugin::Channel& channel)
{
    m_channelId = channel.channelId;

    m_descriptionLabel->setText(channel.description);
    m_metadataLabel->setText(channel.metadataText);
    m_subscribeWidget->setData(channel.subscribeButton);
    m_thumbLabel->setImage(channel.channelAvatarUrl, TubeLabel::LazyLoaded);
    m_titleLabel->setInfo(m_channelId, channel.channelName, channel.channelBadges);
}
