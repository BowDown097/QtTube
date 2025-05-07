#include "browsechannelrenderer.h"
#include "httpreply.h"
#include "innertube/objects/channel/channel.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include "utils/tubeutils.h"
#include <QBoxLayout>
#include <QMessageBox>

BrowseChannelRenderer::BrowseChannelRenderer(QWidget* parent)
    : QWidget(parent),
      descriptionLabel(new TubeLabel(this)),
      hbox(new QHBoxLayout(this)),
      metadataLabel(new TubeLabel(this)),
      subscribeWidget(new SubscribeWidget(this)),
      textVbox(new QVBoxLayout),
      thumbLabel(new TubeLabel(this)),
      titleLabel(new ChannelLabel(this))
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

    connect(thumbLabel, &TubeLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
}

void BrowseChannelRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void BrowseChannelRenderer::setData(const InnertubeObjects::Channel& channel)
{
    this->channelId = channel.channelId;

    titleLabel->setInfo(channelId, channel.title.text, channel.ownerBadges);

    if (channel.descriptionSnippet.text.isEmpty())
    {
        textVbox->removeWidget(descriptionLabel);
        descriptionLabel->deleteLater();
    }
    else
    {
        descriptionLabel->setText(channel.descriptionSnippet.text);
    }

    // "google lied to you!" - kanye west
    // subscriberCountText and videoCountText may be what they say, but they also may not.
    // either one can actually be the channel handle (but never both), so we have to check for that.
    const QString& givenSubCount = channel.subscriberCountText.text;
    const QString& givenVideoCount = channel.videoCountText.text;

    const QString& realSubCount = givenSubCount.startsWith('@') ? givenVideoCount : givenSubCount;
    const QString& handleOrVideos = givenSubCount.startsWith('@') ? givenSubCount : givenVideoCount;

    std::visit([this](auto&& v) { subscribeWidget->setSubscribeButton(v); }, channel.subscribeButton);
    subscribeWidget->setSubscriberCount(realSubCount, channelId);

    TubeUtils::getSubCount(channelId, realSubCount).then([this, handleOrVideos](std::pair<QString, bool> result) {
        if (!metadataLabel)
            return;

        // add "subscribers" if we got full count so the format is consistent
        if (result.second)
            result.first += " subscribers";

        if (!handleOrVideos.isEmpty())
            metadataLabel->setText(QStringLiteral("%1 • %2").arg(result.first, handleOrVideos));
        else
            metadataLabel->setText(result.first);
    });
}

void BrowseChannelRenderer::setThumbnail(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap);
}
