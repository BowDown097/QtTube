#include "channelbrowser.h"
#include "innertube/innertubeexception.h"
#include "innertube/objects/channel/aboutfullmetadata.h"
#include "ui/widgets/labels/tubelabel.h"
#include "uiutilities.h"
#include <QApplication>
#include <QDesktopServices>
#include <QJsonObject>
#include <QUrlQuery>

void ChannelBrowser::setupAbout(QListWidget* channelTab, const QJsonValue& tabRenderer)
{
    QJsonValue metadataRenderer = tabRenderer["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][0]
                                             ["channelAboutFullMetadataRenderer"];
    if (!metadataRenderer.isObject())
        throw InnertubeException("[SetupAbout] channelAboutFullMetadataRenderer not found");

    InnertubeObjects::AboutFullMetadata metadata(metadataRenderer);

    UIUtilities::addBoldLabelToList(channelTab, metadata.viewCountText);
    UIUtilities::addWrappedLabelToList(channelTab, metadata.joinedDateText.text);

    if (metadata.showDescription && !metadata.description.isEmpty())
    {
        UIUtilities::addSeparatorToList(channelTab);
        UIUtilities::addBoldLabelToList(channelTab, metadata.descriptionLabel.text);
        UIUtilities::addWrappedLabelToList(channelTab, metadata.description);
    }

    if (!metadata.country.isEmpty())
    {
        UIUtilities::addSeparatorToList(channelTab);
        UIUtilities::addBoldLabelToList(channelTab, metadata.detailsLabel.text);
        UIUtilities::addWrappedLabelToList(channelTab, metadata.countryLabel.text.trimmed() + " " + metadata.country);
    }

    if (!metadata.primaryLinks.isEmpty())
    {
        UIUtilities::addSeparatorToList(channelTab);
        UIUtilities::addBoldLabelToList(channelTab, metadata.primaryLinksLabel.text);

        for (const InnertubeObjects::ChannelHeaderLink& link : metadata.primaryLinks)
        {
            TubeLabel* label = new TubeLabel(link.title);
            label->setClickable(true, false);
            label->setStyleSheet("color: #167ac6");
            UIUtilities::addWidgetToList(channelTab, label);

            QObject::connect(label, &TubeLabel::clicked, [link]
            {
                QUrl url(link.navigationEndpoint["urlEndpoint"]["url"].toString());
                QUrlQuery query(url);
                QDesktopServices::openUrl(QUrl::fromPercentEncoding(query.queryItemValue("q").toUtf8()));
            });
        }
    }
}

void ChannelBrowser::setupChannels(QListWidget* channelTab, const QJsonValue& tabRenderer)
{
    const QJsonArray contents = tabRenderer["content"]["sectionListRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        const QJsonArray itemSectionContents = v["itemSectionRenderer"]["contents"].toArray();
        for (const QJsonValue& v2 : itemSectionContents)
        {
            if (v2["gridRenderer"].isObject())
            {
                const QJsonArray gridItems = v2["gridRenderer"]["items"].toArray();
                for (const QJsonValue& v3 : gridItems)
                {
                    if (!v3.toObject().contains("gridChannelRenderer"))
                        continue;

                    InnertubeObjects::Channel channel(v3["gridChannelRenderer"]);
                    UIUtilities::addChannelRendererToList(channelTab, channel);
                }
            }
            else if (v2["shelfRenderer"].isObject())
            {
                UIUtilities::addShelfTitleToList(channelTab, v2["shelfRenderer"]);
                QJsonArray shelfItems = v2["shelfRenderer"]["content"]["horizontalListRenderer"]["items"].toArray();
                if (shelfItems.isEmpty()) // if no horizontal list, try expanded contents
                    shelfItems = v2["shelfRenderer"]["content"]["expandedShelfContentsRenderer"]["items"].toArray();

                for (const QJsonValue& v3 : qAsConst(shelfItems))
                {
                    const QJsonObject& obj = v3.toObject();
                    QJsonObject::const_iterator it = obj.begin();
                    InnertubeObjects::Channel channel(it.value());
                    UIUtilities::addChannelRendererToList(channelTab, channel);
                }
            }
        }
    }

    if (channelTab->count() == 0)
    {
        QListWidgetItem* item = new QListWidgetItem("This channel doesn't feature any other channels.", channelTab);
        channelTab->addItem(item);
    }
}

void ChannelBrowser::setupHome(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp)
{
    const QJsonArray contents = tabRenderer["content"]["sectionListRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        const QJsonArray itemSectionContents = v["itemSectionRenderer"]["contents"].toArray();
        for (const QJsonValue& v2 : itemSectionContents)
        {
            if (!v2["shelfRenderer"].isObject())
                continue;

            UIUtilities::addShelfTitleToList(channelTab, v2["shelfRenderer"]);
            QJsonArray list = v2["shelfRenderer"]["content"]["horizontalListRenderer"]["items"].toArray();
            if (list.isEmpty()) // if no horizontal list, try expanded contents
                list = v2["shelfRenderer"]["content"]["expandedShelfContentsRenderer"]["items"].toArray();

            for (const QJsonValue& v3 : qAsConst(list))
            {
                const QJsonObject& obj = v3.toObject();
                QJsonObject::const_iterator it = obj.begin();
                if (it.key() == "channelRenderer" || it.key() == "gridChannelRenderer")
                {
                    InnertubeObjects::Channel channel(it.value());
                    UIUtilities::addChannelRendererToList(channelTab, channel);
                }
                else if (it.key() == "gridVideoRenderer" || it.key() == "videoRenderer")
                {
                    // id and name are missing a lot of the time, so we need to populate them manually
                    InnertubeObjects::Video video(it.value(), it.key() == "gridVideoRenderer");
                    video.owner.id = channelResp.metadata.externalId;
                    video.owner.name = channelResp.metadata.title;
                    UIUtilities::addVideoRendererToList(channelTab, video);
                }
            }
        }
    }
}

void ChannelBrowser::setupLive(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp)
{
    const QJsonArray contents = tabRenderer["content"]["richGridRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        if (!v["richItemRenderer"].isObject())
            continue;

        InnertubeObjects::Video video(v["richItemRenderer"]["content"]["videoRenderer"], false);
        video.owner.id = channelResp.metadata.externalId;
        video.owner.name = channelResp.metadata.title;
        UIUtilities::addVideoRendererToList(channelTab, video);
    }

    if (channelTab->count() == 0)
    {
        QListWidgetItem* item = new QListWidgetItem("This channel has no live streams.", channelTab);
        channelTab->addItem(item);
    }
}

void ChannelBrowser::setupMembership(QListWidget* channelTab, const QJsonValue& tabRenderer)
{
    const QJsonArray slr = tabRenderer["content"]["sectionListRenderer"]["contents"].toArray();
    auto perksIter = std::ranges::find_if(slr, [](const QJsonValue& v) { return v["sponsorshipsExpandablePerksRenderer"].isObject(); });

    if (perksIter != slr.end())
    {
        const QJsonValue& perksParent = *perksIter;
        QJsonValue perks = perksParent["sponsorshipsExpandablePerksRenderer"];

        QWidget* perksHeaderWrapper = new QWidget;
        QHBoxLayout* perksHeader = new QHBoxLayout(perksHeaderWrapper);
        perksHeader->setContentsMargins(0, 0, 0, 0);

        QLabel* badgeLabel = new QLabel;
        badgeLabel->setFixedSize(18, 18);
        UIUtilities::setThumbnail(badgeLabel, perks["badge"]["thumbnails"].toArray());
        perksHeader->addWidget(badgeLabel);

        TubeLabel* membershipTitle = new TubeLabel(InnertubeObjects::InnertubeString(perks["title"]));
        perksHeader->addWidget(membershipTitle);

        UIUtilities::addWidgetToList(channelTab, perksHeaderWrapper);

        QWidget* perkInfoHeaderWrapper = new QWidget;
        QHBoxLayout* perkInfoHeader = new QHBoxLayout(perkInfoHeaderWrapper);
        perkInfoHeader->setContentsMargins(0, 0, 0, 5);

        TubeLabel* tier = new TubeLabel(perks["expandableHeader"]["simpleText"].toString());
        tier->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
        perkInfoHeader->addWidget(tier);

        TubeLabel* showPerkInfo = new TubeLabel("Show perks info");
        showPerkInfo->setClickable(true, false);
        showPerkInfo->setStyleSheet("color: #3ea6ff");
        perkInfoHeader->addWidget(showPerkInfo);

        perkInfoHeader->addStretch();
        UIUtilities::addWidgetToList(channelTab, perkInfoHeaderWrapper);

        QWidget* perkInfoWrapper = new QWidget;
        QVBoxLayout* perkInfo = new QVBoxLayout(perkInfoWrapper);
        perkInfo->setContentsMargins(0, 0, 0, 0);

        QListWidgetItem* perkInfoItem = UIUtilities::addWidgetToList(channelTab, perkInfoWrapper);
        QObject::connect(showPerkInfo, &TubeLabel::clicked, showPerkInfo, [perkInfo, perkInfoItem, perkInfoWrapper, perks, showPerkInfo]
        {
            if (!perkInfo->isEmpty())
            {
                UIUtilities::clearLayout(perkInfo);
                showPerkInfo->setText("Show perks info");
                perkInfoItem->setSizeHint(perkInfoWrapper->sizeHint());
                return;
            }

            showPerkInfo->setText("Hide perks info");

            const QJsonArray expandableItems = perks["expandableItems"].toArray();
            for (const QJsonValue& v : expandableItems)
            {
                QJsonValue perkRenderer = v["sponsorshipsPerkRenderer"];
                if (!perkRenderer.isObject())
                    continue;

                TubeLabel* titleLabel = new TubeLabel(InnertubeObjects::InnertubeString(perkRenderer["title"]));
                titleLabel->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
                perkInfo->addWidget(titleLabel);

                if (perkRenderer["loyaltyBadges"].isObject())
                {
                    const QJsonArray loyaltyBadges = perkRenderer["loyaltyBadges"]["sponsorshipsLoyaltyBadgesRenderer"]["loyaltyBadges"].toArray();
                    for (const QJsonValue& v2 : loyaltyBadges)
                    {
                        QJsonValue loyaltyBadge = v2["sponsorshipsLoyaltyBadgeRenderer"];

                        QWidget* loyaltyBadgeWrapper = new QWidget;
                        loyaltyBadgeWrapper->setFixedWidth(180);

                        QHBoxLayout* loyaltyBadgeLayout = new QHBoxLayout(loyaltyBadgeWrapper);
                        loyaltyBadgeLayout->setContentsMargins(0, 0, 0, 0);

                        TubeLabel* loyaltyBadgeLabel = new TubeLabel(InnertubeObjects::InnertubeString(loyaltyBadge["title"]));
                        loyaltyBadgeLayout->addWidget(loyaltyBadgeLabel);

                        QLabel* loyaltyBadgeIcon = new QLabel;
                        loyaltyBadgeIcon->setFixedSize(18, 18);
                        UIUtilities::setThumbnail(loyaltyBadgeIcon, loyaltyBadge["icon"]["thumbnails"].toArray());
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
                        UIUtilities::setThumbnail(thumbnailLabel, v2["thumbnails"].toArray(), true);
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

    auto itemSectionIter = std::ranges::find_if(slr, [](const QJsonValue& v) { return v["itemSectionRenderer"].isObject(); });
    if (itemSectionIter == slr.end())
        return;

    const QJsonValue& itemSectionRenderer = *itemSectionIter;
    const QJsonArray itemSectionContents = itemSectionRenderer["itemSectionRenderer"]["contents"].toArray();

    for (const QJsonValue& v : itemSectionContents)
    {
        if (!v["videoRenderer"].isObject())
            continue;
        InnertubeObjects::Video video(v["videoRenderer"], false);
        UIUtilities::addVideoRendererToList(channelTab, video);
    }
}

void ChannelBrowser::setupShorts(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp)
{
    const QJsonArray contents = tabRenderer["content"]["richGridRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        if (!v["richItemRenderer"].isObject())
            continue;

        InnertubeObjects::Reel reel(v["richItemRenderer"]["content"]["reelItemRenderer"]);
        reel.owner.id = channelResp.metadata.externalId;
        reel.owner.name = channelResp.metadata.title;
        UIUtilities::addVideoRendererToList(channelTab, reel);
    }

    if (channelTab->count() == 0)
    {
        QListWidgetItem* item = new QListWidgetItem("This channel has no shorts.", channelTab);
        channelTab->addItem(item);
    }
}

void ChannelBrowser::setupUnimplemented(QListWidget* channelTab)
{
    QListWidgetItem* item = new QListWidgetItem("This tab is unimplemented.", channelTab);
    channelTab->addItem(item);
}

void ChannelBrowser::setupVideos(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp)
{
    // TODO: add filtering
    const QJsonArray contents = tabRenderer["content"]["richGridRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        if (!v["richItemRenderer"].isObject())
            continue;

        InnertubeObjects::Video video(v["richItemRenderer"]["content"]["videoRenderer"], false);
        video.owner.id = channelResp.metadata.externalId;
        video.owner.name = channelResp.metadata.title;
        UIUtilities::addVideoRendererToList(channelTab, video);
    }

    if (channelTab->count() == 0)
    {
        QListWidgetItem* item = new QListWidgetItem("This channel has no videos.", channelTab);
        channelTab->addItem(item);
    }
}
