#include "channellabel.h"
#include "badgelabel.h"
#include "ui/views/viewcontroller.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QMenu>

ChannelLabel::ChannelLabel(PluginEntry* plugin, QWidget* parent)
    : QWidget(parent), text(new TubeLabel(this)), m_badgeLayout(new QHBoxLayout), m_layout(new QHBoxLayout(this))
{
    text->setClickable(true);
    text->setContextMenuPolicy(Qt::CustomContextMenu);
    text->setUnderlineOnHover(true);

    m_layout->addWidget(text);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_badgeLayout->setSpacing(2);
    m_layout->addLayout(m_badgeLayout);

    connect(text, &TubeLabel::clicked, this, [this, plugin] { ViewController::loadChannel(m_channelId, plugin); });
    connect(text, &TubeLabel::customContextMenuRequested, this, &ChannelLabel::showContextMenu);
}

void ChannelLabel::addStretch()
{
    m_layout->addStretch();
}

void ChannelLabel::copyChannelUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/channel/" + m_channelId);
}

void ChannelLabel::reset()
{
    text->clear();
    UIUtils::clearLayout(m_badgeLayout);
}

void ChannelLabel::setInfo(const QString& uploaderId, const QString& uploaderName, const QList<QtTubePlugin::Badge>& badges)
{
    setInfo(uploaderId, uploaderName);

    for (const QtTubePlugin::Badge& badge : badges)
    {
        BadgeLabel* badgeLabel = new BadgeLabel(this);
        badgeLabel->setFixedSize(13, 10);
        badgeLabel->setData(badge);
        m_badgeLayout->addWidget(badgeLabel);
    }

    if (m_badgeLayout->count() > 0)
        m_badgeLayout->addStretch();
}

void ChannelLabel::setInfo(const QString& uploaderId, const QString& uploaderName)
{
    reset();
    m_channelId = uploaderId;

    text->setClickable(!uploaderId.isEmpty());
    text->setText(uploaderName);
    text->setUnderlineOnHover(!uploaderId.isEmpty());
}

void ChannelLabel::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &ChannelLabel::copyChannelUrl);

    menu->addAction(copyUrlAction);
    menu->popup(mapToGlobal(pos));
}
