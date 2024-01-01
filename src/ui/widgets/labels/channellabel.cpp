#include "channellabel.h"
#include "channelbadgelabel.h"
#include "innertube/objects/channel/metadatabadge.h"
#include <QBoxLayout>

ChannelLabel::ChannelLabel(QWidget* parent)
    : QWidget(parent), text(new TubeLabel(this)), layout(new QHBoxLayout(this))
{
    text->setClickable(true, true);
    text->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(text);
    layout->setContentsMargins(0, 0, 0, 0);
}

void ChannelLabel::reset()
{
    text->clear();
    for (int i = 0; i < layout->count(); i++)
    {
        QLayoutItem* item = layout->itemAt(i);
        if (QWidget* widget = item->widget(); !qobject_cast<TubeLabel*>(widget))
            widget->deleteLater();
        layout->removeItem(item);
        delete item;
    }
}

void ChannelLabel::setInfo(const QString& channelName, const QList<InnertubeObjects::MetadataBadge>& badges)
{
    text->setText(channelName);
    text->adjustSize();

    for (const InnertubeObjects::MetadataBadge& badge : badges)
    {
        ChannelBadgeLabel* badgeLabel = new ChannelBadgeLabel(this);
        badgeLabel->setData(badge);
        layout->addSpacing(2);
        layout->addWidget(badgeLabel);
    }

    layout->addStretch();
}
