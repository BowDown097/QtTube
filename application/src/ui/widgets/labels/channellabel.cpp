#include "channellabel.h"
#include "channelbadgelabel.h"
#include "ui/views/viewcontroller.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QMenu>

ChannelLabel::ChannelLabel(QWidget* parent)
    : QWidget(parent), text(new TubeLabel(this)), badgeLayout(new QHBoxLayout), layout(new QHBoxLayout(this))
{
    text->setClickable(true);
    text->setContextMenuPolicy(Qt::CustomContextMenu);
    text->setUnderlineOnHover(true);

    layout->addWidget(text);
    layout->setContentsMargins(0, 0, 0, 0);

    badgeLayout->setSpacing(2);
    layout->addLayout(badgeLayout);

    connect(text, &TubeLabel::clicked, this, &ChannelLabel::navigate);
    connect(text, &TubeLabel::customContextMenuRequested, this, &ChannelLabel::showContextMenu);
}

void ChannelLabel::addStretch()
{
    layout->addStretch();
}

void ChannelLabel::copyChannelUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/channel/" + channelId);
}

void ChannelLabel::navigate()
{
    ViewController::loadChannel(channelId);
}

void ChannelLabel::reset()
{
    text->clear();
    UIUtils::clearLayout(badgeLayout);
}

void ChannelLabel::setInfo(const QString& uploaderId, const QString& uploaderName, const QList<QtTubePlugin::Badge>& badges)
{
    setInfo(uploaderId, uploaderName);

    for (const QtTubePlugin::Badge& badge : badges)
    {
        ChannelBadgeLabel* badgeLabel = new ChannelBadgeLabel(this);
        badgeLabel->setData(badge);
        badgeLayout->addWidget(badgeLabel);
    }

    if (badgeLayout->count() > 0)
        badgeLayout->addStretch();
}

void ChannelLabel::setInfo(const QString& uploaderId, const QString& uploaderName)
{
    this->channelId = uploaderId;
    reset();
    text->setText(uploaderName);
}

void ChannelLabel::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &ChannelLabel::copyChannelUrl);

    menu->addAction(copyUrlAction);
    menu->popup(mapToGlobal(pos));
}
