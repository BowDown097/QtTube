#include "browsenotificationrenderer.h"
#include "http.h"
#include "qttube-plugin/objects/notification.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

BrowseNotificationRenderer::BrowseNotificationRenderer(QWidget* parent)
    : QWidget(parent),
      bodyLabel(new TubeLabel(this)),
      channelIconLabel(new TubeLabel(this)),
      hbox(new QHBoxLayout(this)),
      sentTimeLabel(new TubeLabel(this)),
      textVbox(new QVBoxLayout),
      thumbLabel(new TubeLabel(this))
{
    bodyLabel->setWordWrap(true);

    textVbox->addStretch();
    textVbox->addWidget(bodyLabel);
    textVbox->addWidget(sentTimeLabel);
    textVbox->addStretch();

    channelIconLabel->setFixedSize(48, 48);
    channelIconLabel->setScaledContents(true);
    hbox->addWidget(channelIconLabel);

    hbox->addLayout(textVbox, 1);

    thumbLabel->setFixedSize(128, 72);
    thumbLabel->setScaledContents(true);
    hbox->addWidget(thumbLabel);

    sentTimeLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
}

void BrowseNotificationRenderer::setChannelIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIconLabel->setPixmap(pixmap);
}

void BrowseNotificationRenderer::setData(const QtTube::PluginNotification& notification)
{
    sentTimeLabel->setText(notification.sentTimeText);
    bodyLabel->setText(notification.body);

    HttpReply* iconReply = Http::instance().get(notification.channelAvatarUrl);
    connect(iconReply, &HttpReply::finished, this, &BrowseNotificationRenderer::setChannelIcon);

    HttpReply* thumbReply = Http::instance().get(notification.thumbnailUrl);
    connect(thumbReply, &HttpReply::finished, this, &BrowseNotificationRenderer::setThumbnail);
}

void BrowseNotificationRenderer::setThumbnail(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap);
}
