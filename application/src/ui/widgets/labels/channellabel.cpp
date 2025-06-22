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
    if (!channelId.isEmpty())
        ViewController::loadChannel(channelId);
    else if (const QJsonValue urlEndpoint = channelEndpoint["urlEndpoint"]; urlEndpoint.isObject())
        QDesktopServices::openUrl(urlEndpoint["url"].toString());
}

void ChannelLabel::reset()
{
    text->clear();
    UIUtils::clearLayout(badgeLayout);
}

void ChannelLabel::setInfo(const QString& channelId, const QString& channelName,
                           const QList<InnertubeObjects::MetadataBadge>& badges)
{
    setInfo(channelId, channelName);

    for (const InnertubeObjects::MetadataBadge& badge : badges)
    {
        ChannelBadgeLabel* badgeLabel = new ChannelBadgeLabel(this);
        badgeLabel->setData(badge);
        badgeLayout->addWidget(badgeLabel);
    }

    if (badgeLayout->count() > 0)
        badgeLayout->addStretch();
}

void ChannelLabel::setInfo(const QString& uploaderId, const QString& uploaderName, const QList<QtTube::PluginBadge>& badges)
{
    setInfo(uploaderId, uploaderName);

    for (const QtTube::PluginBadge& badge : badges)
    {
        ChannelBadgeLabel* badgeLabel = new ChannelBadgeLabel(this);
        badgeLabel->setData(badge);
        badgeLayout->addWidget(badgeLabel);
    }

    if (badgeLayout->count() > 0)
        badgeLayout->addStretch();
}

void ChannelLabel::setInfo(const QJsonValue& endpoint, const QString& name)
{
    this->channelEndpoint = endpoint;

    reset();
    text->setText(name);
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
