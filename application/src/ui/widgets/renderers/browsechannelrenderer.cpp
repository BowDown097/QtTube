#include "browsechannelrenderer.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include <QBoxLayout>
#include <QMessageBox>

BrowseChannelRenderer::BrowseChannelRenderer(PluginData* plugin, QWidget* parent)
    : QWidget(parent),
      descriptionLabel(new TubeLabel(this)),
      hbox(new QHBoxLayout(this)),
      metadataLabel(new TubeLabel(this)),
      subscribeWidget(new SubscribeWidget(plugin, this)),
      textVbox(new QVBoxLayout),
      thumbLabel(new TubeLabel(this)),
      titleLabel(new ChannelLabel(plugin, this))
{
    titleLabel->text->setFont(QFont(font().toString(), font().pointSize() + 2, QFont::Bold));
    titleLabel->addStretch();
    textVbox->addWidget(titleLabel);

    textVbox->addWidget(metadataLabel);

    descriptionLabel->setMaximumLines(2);
    descriptionLabel->setWordWrap(true);
    textVbox->addWidget(descriptionLabel);

    subscribeWidget->layout->addStretch();
    textVbox->addWidget(subscribeWidget);

    thumbLabel->setClickable(true);
    thumbLabel->setFixedSize(80, 80);
    thumbLabel->setScaledContents(true);
    hbox->addWidget(thumbLabel);

    hbox->addLayout(textVbox);

    connect(thumbLabel, &TubeLabel::clicked, this, [this, plugin] { ViewController::loadChannel(channelId, plugin); });
}

void BrowseChannelRenderer::setData(const QtTubePlugin::Channel& channel)
{
    this->channelId = channel.channelId;
    subscribeWidget->setData(channel.subscribeButton);
    thumbLabel->setImage(channel.channelAvatarUrl, TubeLabel::LazyLoaded);
    titleLabel->setInfo(channelId, channel.channelName, channel.channelBadges);

    if (channel.description.isEmpty())
    {
        textVbox->removeWidget(descriptionLabel);
        descriptionLabel->deleteLater();
    }
    else
    {
        descriptionLabel->setText(channel.description);
    }

    if (channel.metadataText.isEmpty())
    {
        textVbox->removeWidget(metadataLabel);
        metadataLabel->deleteLater();
    }
    else
    {
        metadataLabel->setText(channel.metadataText);
    }
}
