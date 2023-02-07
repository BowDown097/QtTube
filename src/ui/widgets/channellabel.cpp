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
}

void ChannelLabel::setInfo(const QString& channelName, const QList<InnertubeObjects::MetadataBadge>& badges)
{
    text->setText(channelName);

    for (const InnertubeObjects::MetadataBadge& badge : badges)
    {
        layout->addSpacing(2);
        QLabel* badgeLabel = new QLabel(this);
        badgeLabel->setText("✔");
        badgeLabel->setToolTip(badge.tooltip);
        layout->addWidget(badgeLabel);
    }

    layout->addStretch();
}
