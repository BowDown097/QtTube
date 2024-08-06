#include "browsenotificationrenderer.h"
#include "httpreply.h"
#include "innertube/objects/notification/notification.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

BrowseNotificationRenderer::BrowseNotificationRenderer(QWidget* parent)
    : QWidget(parent),
      channelIcon(new TubeLabel(this)),
      hbox(new QHBoxLayout(this)),
      sentTimeText(new TubeLabel(this)),
      shortMessage(new TubeLabel(this)),
      textVbox(new QVBoxLayout),
      thumbLabel(new TubeLabel(this))
{
    channelIcon->setFixedSize(48, 48);
    thumbLabel->setFixedSize(128, 72);

    shortMessage->setWordWrap(true);

    textVbox->addStretch();
    textVbox->addWidget(shortMessage);
    textVbox->addWidget(sentTimeText);
    textVbox->addStretch();

    hbox->addWidget(channelIcon);
    hbox->addLayout(textVbox, 1);
    hbox->addWidget(thumbLabel);

    sentTimeText->setFont(QFont(font().toString(), font().pointSize() - 2));
}

void BrowseNotificationRenderer::setChannelIcon(const HttpReply& reply)
{
    if (reply.statusCode() != 200)
        return;
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void BrowseNotificationRenderer::setData(const InnertubeObjects::Notification& notification)
{
    sentTimeText->setText(notification.sentTimeText);
    shortMessage->setText(notification.shortMessage);
}

void BrowseNotificationRenderer::setThumbnail(const HttpReply& reply)
{
    if (reply.statusCode() != 200)
        return;
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap.scaled(128, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
