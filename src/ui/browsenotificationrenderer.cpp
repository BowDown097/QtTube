#include "browsenotificationrenderer.h"
#include <QApplication>

BrowseNotificationRenderer::BrowseNotificationRenderer(QWidget* parent) : QWidget(parent)
{
    channelIcon = new TubeLabel;
    hbox = new QHBoxLayout;
    sentTimeText = new TubeLabel;
    shortMessage = new TubeLabel;
    textVbox = new QVBoxLayout;
    thumbLabel = new TubeLabel;

    channelIcon->setFixedSize(48, 48);
    thumbLabel->setFixedHeight(72);

    shortMessage->setWordWrap(true);

    textVbox->addStretch();
    textVbox->addWidget(shortMessage);
    textVbox->addWidget(sentTimeText);
    textVbox->addStretch();

    hbox->addWidget(channelIcon);
    hbox->addLayout(textVbox, 1);
    hbox->addWidget(thumbLabel);
    setLayout(hbox);

    sentTimeText->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() - 2));
}

void BrowseNotificationRenderer::setChannelIcon(const HttpReply& reply)
{
    if (reply.statusCode() == 404)
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
    thumbLabel->setPixmap(pixmap.scaledToWidth(128, Qt::SmoothTransformation));
}
