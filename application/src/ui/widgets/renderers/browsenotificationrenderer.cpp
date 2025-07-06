#include "browsenotificationrenderer.h"
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

void BrowseNotificationRenderer::setData(const QtTubePlugin::Notification& notification)
{
    bodyLabel->setText(notification.body);
    channelIconLabel->setImage(notification.channelAvatarUrl);
    sentTimeLabel->setText(notification.sentTimeText);
    thumbLabel->setImage(notification.thumbnailUrl);
}
