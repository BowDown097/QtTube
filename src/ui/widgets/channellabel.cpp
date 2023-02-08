#include "channellabel.h"

ChannelLabel::ChannelLabel(QWidget* parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    text = new TubeLabel(this);
    text->setClickable(true, true);
    text->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(text);

    layout->addSpacing(2);

    badgeLabel = new QLabel(this);
    layout->addWidget(badgeLabel);

    layout->addStretch();
}

void ChannelLabel::setInfo(const QString& channelName, const QList<InnertubeObjects::MetadataBadge>& badges)
{
    text->setText(channelName);
    if (badges.size() > 0)
    {
        badgeLabel->setText("✔");
        badgeLabel->setToolTip(badges[0].tooltip);
    }
    else
    {
        badgeLabel->setText(QString());
        badgeLabel->setToolTip(QString());
    }
}
