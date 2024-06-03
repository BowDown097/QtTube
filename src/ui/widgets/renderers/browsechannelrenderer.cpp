#include "browsechannelrenderer.h"
#include "httpreply.h"
#include "innertube/objects/channel/channel.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include <QBoxLayout>
#include <QMessageBox>
#include <QtNetwork/QtNetwork>

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

    descriptionLabel->setWordWrap(true);
    UIUtils::setMaximumLines(descriptionLabel, 2);
    textVbox->addWidget(descriptionLabel);

    subscribeWidget->layout->addStretch();
    textVbox->addWidget(subscribeWidget);

    thumbLabel->setClickable(true, false);
    thumbLabel->setFixedSize(80, 80);
    hbox->addWidget(thumbLabel);

    hbox->addLayout(textVbox);

    connect(thumbLabel, &TubeLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
    connect(titleLabel->text, &TubeLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
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

    QString subCount = channel.subscriberCountText.text;
    QString videoCount = channel.videoCountText.text;

    subscribeWidget->setSubscribeButton(channel.subscribeButton);
    subscribeWidget->setSubscriberCount(subCount.contains("subscribers") ? subCount : videoCount, channelId);

    if (qtTubeApp->settings().fullSubs)
    {
        // QNetworkAccessManager needs to be used here due to a bug with the http library
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        manager->setTransferTimeout(2000);

        QNetworkReply* reply = manager->get(QNetworkRequest("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelId));
        connect(reply, &QNetworkReply::finished, this, [this, reply, subCount, videoCount] {
            reply->deleteLater();
            reply->manager()->deleteLater();

            if (reply->error() != QNetworkReply::NoError)
                return;

            int subs = QJsonDocument::fromJson(reply->readAll())["est_sub"].toInt();
            QString fullSubs = QLocale::system().toString(subs) + " subscribers";

            // in some cases, subCount is the channel handle, and other times it's actually the sub count.
            // thanks innertube! so ya, we have to check for that, otherwise subs show up twice.
            metadataLabel->setText(subCount.contains(" subscribers")
                ? QStringLiteral("%1 • %2").arg(fullSubs, videoCount) : QStringLiteral("%1 • %2").arg(subCount, fullSubs));
        });
    }
    else
    {
        metadataLabel->setText(!videoCount.isEmpty() ? QStringLiteral("%1 • %2").arg(subCount, videoCount) : subCount);
    }
}

void BrowseChannelRenderer::setThumbnail(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
