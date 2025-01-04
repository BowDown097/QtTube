#include "channellabel.h"
#include "channelbadgelabel.h"
#include "innertube.h"
#include "qttubeapplication.h"
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

void ChannelLabel::filterThis()
{
    if (qtTubeApp->settings().channelIsFiltered(channelId))
        return;

    auto channel = InnerTube::instance()->getBlocking<InnertubeEndpoints::BrowseChannel>(channelId);

    QString channelHandle;
    if (auto c4 = std::get_if<InnertubeObjects::ChannelC4Header>(&channel.response.header))
    {
        channelHandle = c4->channelHandleText.text;
    }
    else if (auto page = std::get_if<InnertubeObjects::ChannelPageHeader>(&channel.response.header))
    {
        const QList<QList<InnertubeObjects::DynamicText>> metadataRows = page->metadata.metadataRows;
        if (!metadataRows.empty() && !metadataRows[0].empty())
            channelHandle = metadataRows[0][0].content;
    }

    qtTubeApp->settings().filteredChannels.append(channelId + "|" + channelHandle);
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
    this->channelId = channelId;

    reset();
    text->setText(channelName);

    for (const InnertubeObjects::MetadataBadge& badge : badges)
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

void ChannelLabel::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &ChannelLabel::copyChannelUrl);

    QAction* filterAction = new QAction("Filter this channel", this);
    connect(filterAction, &QAction::triggered, this, &ChannelLabel::filterThis);

    menu->addAction(copyUrlAction);
    menu->addAction(filterAction);
    menu->popup(mapToGlobal(pos));
}
