#include "channelbrowser.h"
#include "innertube/innertubeexception.h"
#include "innertube/objects/backstage/backstagepost.h"
#include "innertube/objects/channel/aboutfullmetadata.h"
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "innertube/responses/browse/channelresponse.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>

void ChannelBrowser::setupAbout(QListWidget* widget, const QJsonValue& renderer)
{
    const QJsonValue metadataRenderer = renderer["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]
                                                ["contents"][0]["channelAboutFullMetadataRenderer"];
    if (!metadataRenderer.isObject())
        throw InnertubeException("[SetupAbout] channelAboutFullMetadataRenderer not found");

    InnertubeObjects::AboutFullMetadata metadata(metadataRenderer);

    UIUtils::addBoldLabelToList(widget, metadata.viewCountText);
    UIUtils::addWrappedLabelToList(widget, metadata.joinedDateText.text);

    if (metadata.showDescription && !metadata.description.isEmpty())
    {
        UIUtils::addSeparatorToList(widget);
        UIUtils::addBoldLabelToList(widget, metadata.descriptionLabel.text);
        UIUtils::addWrappedLabelToList(widget, metadata.description);
    }

    if (!metadata.country.isEmpty())
    {
        UIUtils::addSeparatorToList(widget);
        UIUtils::addBoldLabelToList(widget, metadata.detailsLabel.text);
        UIUtils::addWrappedLabelToList(widget, metadata.countryLabel.text.trimmed() + " " + metadata.country);
    }

    if (!metadata.primaryLinks.isEmpty())
    {
        UIUtils::addSeparatorToList(widget);
        UIUtils::addBoldLabelToList(widget, metadata.primaryLinksLabel.text);

        for (const InnertubeObjects::ChannelHeaderLink& link : metadata.primaryLinks)
        {
            TubeLabel* label = new TubeLabel(link.title);
            label->setClickable(true);
            label->setStyleSheet("color: #167ac6");
            UIUtils::addWidgetToList(widget, label);

            QObject::connect(label, &TubeLabel::clicked, [link]
            {
                QUrl url(link.navigationEndpoint["urlEndpoint"]["url"].toString());
                QUrlQuery query(url);
                QDesktopServices::openUrl(QUrl::fromPercentEncoding(query.queryItemValue("q").toUtf8()));
            });
        }
    }
}

void ChannelBrowser::setupChannels(QListWidget* widget, const QJsonValue& renderer)
{
    const QJsonArray contents = renderer["content"]["sectionListRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        const QJsonArray itemSectionContents = v["itemSectionRenderer"]["contents"].toArray();
        for (const QJsonValue& v2 : itemSectionContents)
        {
            if (v2["gridRenderer"].isObject())
            {
                const QJsonArray items = v2["gridRenderer"]["items"].toArray();
                for (const QJsonValue& v3 : items)
                {
                    if (!v3["gridChannelRenderer"].isObject())
                        continue;

                    InnertubeObjects::Channel channel(v3["gridChannelRenderer"]);
                    UIUtils::addChannelRendererToList(widget, channel);
                }
            }
            else if (v2["shelfRenderer"].isObject())
            {
                UIUtils::addShelfTitleToList(widget, v2["shelfRenderer"]);
                const QJsonValue content = v2["shelfRenderer"]["content"];
                const QJsonArray items = content["horizontalListRenderer"].isObject()
                    ? content["horizontalListRenderer"]["items"].toArray()
                    : content["expandedShelfContentsRenderer"]["items"].toArray();

                for (const QJsonValue& v3 : items)
                {
                    const QJsonObject& obj = v3.toObject();
                    QJsonObject::const_iterator it = obj.begin();
                    InnertubeObjects::Channel channel(it.value());
                    UIUtils::addChannelRendererToList(widget, channel);
                }
            }
        }
    }

    if (widget->count() == 0)
        widget->addItem("This channel doesn't feature any other channels.");
}

void ChannelBrowser::setupCommunity(QListWidget* widget, const QJsonValue& renderer)
{
    const QJsonArray contents = renderer["content"]["sectionListRenderer"]["contents"][0]
                                        ["itemSectionRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        InnertubeObjects::BackstagePost post(v["backstagePostThreadRenderer"]["post"]["backstagePostRenderer"]);
        UIUtils::addBackstagePostToList(widget, post);
    }

    if (widget->count() == 0)
        widget->addItem("This channel hasn't posted yet.");
}

void ChannelBrowser::setupHome(QListWidget* widget, const QJsonValue& renderer,
                               const InnertubeEndpoints::ChannelResponse& resp)
{
    const QJsonArray contents = renderer["content"]["sectionListRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        const QJsonArray itemSectionContents = v["itemSectionRenderer"]["contents"].toArray();
        for (const QJsonValue& v2 : itemSectionContents)
        {
            if (!v2["shelfRenderer"].isObject())
                continue;

            UIUtils::addShelfTitleToList(widget, v2["shelfRenderer"]);
            const QJsonValue content = v2["shelfRenderer"]["content"];
            const QJsonArray items = content["horizontalListRenderer"].isObject()
                ? content["horizontalListRenderer"]["items"].toArray()
                : content["expandedShelfContentsRenderer"]["items"].toArray();

            for (const QJsonValue& v3 : items)
            {
                const QJsonObject& obj = v3.toObject();
                QJsonObject::const_iterator it = obj.begin();
                if (it.key() == "channelRenderer" || it.key() == "gridChannelRenderer")
                {
                    InnertubeObjects::Channel channel(it.value());
                    UIUtils::addChannelRendererToList(widget, channel);
                }
                else if (it.key() == "gridVideoRenderer" || it.key() == "videoRenderer")
                {
                    // id and name are missing a lot of the time, so we need to populate them manually
                    InnertubeObjects::Video video(it.value());
                    video.owner.id = resp.metadata.externalId;
                    video.owner.name = resp.metadata.title;
                    UIUtils::addVideoRendererToList(widget, video);
                }
            }
        }
    }
}

void ChannelBrowser::setupLive(QListWidget* widget, const QJsonValue& renderer,
                               const InnertubeEndpoints::ChannelResponse& resp)
{
    if (qtTubeApp->settings().hideStreams)
    {
        widget->addItem("This tab is disabled because the live streams filter is turned on.");
        return;
    }

    const QJsonArray contents = renderer["content"]["richGridRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        if (!v["richItemRenderer"].isObject())
            continue;

        InnertubeObjects::Video video(v["richItemRenderer"]["content"]["videoRenderer"]);
        video.owner.id = resp.metadata.externalId;
        video.owner.name = resp.metadata.title;
        UIUtils::addVideoRendererToList(widget, video);
    }

    if (widget->count() == 0)
        widget->addItem("This channel has no live streams.");
}

void ChannelBrowser::setupMembership(QListWidget* widget, const QJsonValue& renderer)
{
    const QJsonArray slr = renderer["content"]["sectionListRenderer"]["contents"].toArray();
    auto perksIter = std::ranges::find_if(slr, [](const QJsonValue& v) {
        return v["sponsorshipsExpandablePerksRenderer"].isObject();
    });

    if (perksIter != slr.end())
    {
        const QJsonValue& perksParent = *perksIter;
        const QJsonValue perks = perksParent["sponsorshipsExpandablePerksRenderer"];

        QWidget* perksHeaderWrapper = new QWidget;
        QHBoxLayout* perksHeader = new QHBoxLayout(perksHeaderWrapper);
        perksHeader->setContentsMargins(0, 0, 0, 0);

        QLabel* badgeLabel = new QLabel;
        badgeLabel->setFixedSize(18, 18);
        UIUtils::setThumbnail(badgeLabel, perks["badge"]["thumbnails"].toArray());
        perksHeader->addWidget(badgeLabel);

        TubeLabel* membershipTitle = new TubeLabel(InnertubeObjects::InnertubeString(perks["title"]));
        perksHeader->addWidget(membershipTitle);

        UIUtils::addWidgetToList(widget, perksHeaderWrapper);

        QWidget* perkInfoHeaderWrapper = new QWidget;
        QHBoxLayout* perkInfoHeader = new QHBoxLayout(perkInfoHeaderWrapper);
        perkInfoHeader->setContentsMargins(0, 0, 0, 5);

        TubeLabel* tier = new TubeLabel(perks["expandableHeader"]["simpleText"].toString());
        tier->setFont(QFont(tier->font().toString(), -1, QFont::Bold));
        perkInfoHeader->addWidget(tier);

        TubeLabel* showPerkInfo = new TubeLabel("Show perks info");
        showPerkInfo->setClickable(true);
        showPerkInfo->setStyleSheet("color: #3ea6ff");
        perkInfoHeader->addWidget(showPerkInfo);

        perkInfoHeader->addStretch();
        UIUtils::addWidgetToList(widget, perkInfoHeaderWrapper);

        QWidget* perkInfoWrapper = new QWidget;
        QVBoxLayout* perkInfo = new QVBoxLayout(perkInfoWrapper);
        perkInfo->setContentsMargins(0, 0, 0, 0);

        QListWidgetItem* perkInfoItem = UIUtils::addWidgetToList(widget, perkInfoWrapper);
        QObject::connect(showPerkInfo, &TubeLabel::clicked, showPerkInfo, [perkInfo, perkInfoItem, perkInfoWrapper, perks, showPerkInfo]
        {
            if (!perkInfo->isEmpty())
            {
                UIUtils::clearLayout(perkInfo);
                showPerkInfo->setText("Show perks info");
                perkInfoItem->setSizeHint(perkInfoWrapper->sizeHint());
                return;
            }

            showPerkInfo->setText("Hide perks info");

            const QJsonArray expandableItems = perks["expandableItems"].toArray();
            for (const QJsonValue& v : expandableItems)
            {
                const QJsonValue perkRenderer = v["sponsorshipsPerkRenderer"];
                if (!perkRenderer.isObject())
                    continue;

                TubeLabel* titleLabel = new TubeLabel(InnertubeObjects::InnertubeString(perkRenderer["title"]));
                titleLabel->setFont(QFont(titleLabel->font().toString(), -1, QFont::Bold));
                perkInfo->addWidget(titleLabel);

                if (perkRenderer["loyaltyBadges"].isObject())
                {
                    const QJsonArray loyaltyBadges = perkRenderer["loyaltyBadges"]["sponsorshipsLoyaltyBadgesRenderer"]
                                                                 ["loyaltyBadges"].toArray();
                    for (const QJsonValue& v2 : loyaltyBadges)
                    {
                        const QJsonValue loyaltyBadge = v2["sponsorshipsLoyaltyBadgeRenderer"];

                        QWidget* loyaltyBadgeWrapper = new QWidget;
                        loyaltyBadgeWrapper->setFixedWidth(180);

                        QHBoxLayout* loyaltyBadgeLayout = new QHBoxLayout(loyaltyBadgeWrapper);
                        loyaltyBadgeLayout->setContentsMargins(0, 0, 0, 0);

                        TubeLabel* loyaltyBadgeLabel = new TubeLabel(InnertubeObjects::InnertubeString(loyaltyBadge["title"]));
                        loyaltyBadgeLayout->addWidget(loyaltyBadgeLabel);

                        QLabel* loyaltyBadgeIcon = new QLabel;
                        loyaltyBadgeIcon->setFixedSize(18, 18);
                        UIUtils::setThumbnail(loyaltyBadgeIcon, loyaltyBadge["icon"]["thumbnails"].toArray());
                        loyaltyBadgeLayout->addWidget(loyaltyBadgeIcon);

                        perkInfo->addWidget(loyaltyBadgeWrapper);
                    }
                }
                else if (perkRenderer["images"].isArray())
                {
                    QWidget* imagesWrapper = new QWidget;
                    QHBoxLayout* imagesLayout = new QHBoxLayout(imagesWrapper);
                    imagesLayout->setContentsMargins(0, 0, 0, 0);

                    const QJsonArray images = perkRenderer["images"].toArray();
                    for (const QJsonValue& v2 : images)
                    {
                        QLabel* thumbnailLabel = new QLabel;
                        thumbnailLabel->setFixedSize(32, 32);
                        UIUtils::setThumbnail(thumbnailLabel, v2["thumbnails"].toArray(), true);
                        imagesLayout->addWidget(thumbnailLabel);
                    }

                    imagesLayout->addStretch();
                    perkInfo->addWidget(imagesWrapper);
                }
                else if (perkRenderer["description"].isObject())
                {
                    TubeLabel* descriptionLabel = new TubeLabel(perkRenderer["description"]["simpleText"].toString());
                    perkInfo->addWidget(descriptionLabel);
                }
            }

            perkInfoItem->setSizeHint(perkInfoWrapper->sizeHint());
        });
    }

    auto itemSectionIter = std::ranges::find_if(slr, [](const QJsonValue& v) {
        return v["itemSectionRenderer"].isObject();
    });
    if (itemSectionIter == slr.end())
        return;

    const QJsonValue& itemSectionRenderer = *itemSectionIter;
    const QJsonArray itemSectionContents = itemSectionRenderer["itemSectionRenderer"]["contents"].toArray();

    for (const QJsonValue& v : itemSectionContents)
    {
        if (!v["videoRenderer"].isObject())
            continue;
        InnertubeObjects::Video video(v["videoRenderer"]);
        UIUtils::addVideoRendererToList(widget, video);
    }
}

void ChannelBrowser::setupShorts(QListWidget* widget, const QJsonValue& renderer,
                                 const InnertubeEndpoints::ChannelResponse& resp)
{
    if (qtTubeApp->settings().hideShorts)
    {
        widget->addItem("This tab is disabled because the shorts filter is turned on.");
        return;
    }

    const QJsonArray contents = renderer["content"]["richGridRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
        if (const QJsonValue richItem = v["richItemRenderer"]; richItem.isObject())
            UIUtils::addVideoRendererToList(widget, InnertubeObjects::Reel(richItem["content"]["reelItemRenderer"]));

    if (widget->count() == 0)
        widget->addItem("This channel has no shorts.");
}

void ChannelBrowser::setupUnimplemented(QListWidget* widget)
{
    widget->addItem("This tab is unimplemented.");
}

void ChannelBrowser::setupVideos(QListWidget* widget, const QJsonValue& renderer,
                                 const InnertubeEndpoints::ChannelResponse& resp)
{
    // TODO: add filtering
    const QJsonArray contents = renderer["content"]["richGridRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        if (!v["richItemRenderer"].isObject())
            continue;

        InnertubeObjects::Video video(v["richItemRenderer"]["content"]["videoRenderer"]);
        video.owner.id = resp.metadata.externalId;
        video.owner.name = resp.metadata.title;
        UIUtils::addVideoRendererToList(widget, video);
    }

    if (widget->count() == 0)
        widget->addItem("This channel has no videos.");
}
