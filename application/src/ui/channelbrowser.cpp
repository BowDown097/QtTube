#include "channelbrowser.h"
#include "innertube/objects/backstage/backstagepost.h"
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "innertube/objects/viewmodels/shortslockupviewmodel.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QJsonArray>
#include <QJsonObject>

namespace ChannelBrowser
{
    void continuation(ContinuableListWidget* widget, const QJsonValue& contents)
    {
        const QJsonArray contentsArr = contents.toArray();
        for (const QJsonValue& item : contentsArr)
        {
            if (const QJsonValue richItem = item["richItemRenderer"]; richItem.isObject())
            {
                const QJsonObject content = richItem["content"].toObject();
                QJsonObject::const_iterator it = content.begin();
                if (it.key() == "gridVideoRenderer" || it.key() == "videoRenderer")
                    UIUtils::addVideoToList(widget, InnertubeObjects::Video(it.value()));
                else if (it.key() == "reelItemRenderer")
                    UIUtils::addVideoToList(widget, InnertubeObjects::Reel(it.value()));
                else if (it.key() == "shortsLockupViewModel")
                    UIUtils::addVideoToList(widget, InnertubeObjects::ShortsLockupViewModel(it.value()));
            }
            else if (const QJsonValue post = item["backstagePostThreadRenderer"]["post"]; post.isObject())
            {
                UIUtils::addBackstagePostToList(widget, InnertubeObjects::BackstagePost(post["backstagePostRenderer"]));
            }
            else if (const QJsonValue continuation = item["continuationItemRenderer"]; continuation.isObject())
            {
                widget->continuationToken = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            }

            QCoreApplication::processEvents();
        }
    }

    void setupCommunity(ContinuableListWidget* widget, const QJsonValue& renderer)
    {
        const QJsonArray contents = renderer["content"]["sectionListRenderer"]["contents"][0]
                                            ["itemSectionRenderer"]["contents"].toArray();
        for (const QJsonValue& v : contents)
        {
            if (const QJsonValue post = v["backstagePostThreadRenderer"]["post"]; post.isObject())
                UIUtils::addBackstagePostToList(widget, InnertubeObjects::BackstagePost(post["backstagePostRenderer"]));
            else if (const QJsonValue continuation = v["continuationItemRenderer"]; continuation.isObject())
                widget->continuationToken = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            QCoreApplication::processEvents();
        }

        if (widget->count() == 0)
            widget->addItem("This channel hasn't posted yet.");
    }

    void setupHome(ContinuableListWidget* widget, const QJsonValue& renderer)
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
                    const QJsonObject obj = v3.toObject();
                    QJsonObject::const_iterator it = obj.begin();
                    if (it.key() == "channelRenderer" || it.key() == "gridChannelRenderer")
                        UIUtils::addChannelToList(widget, InnertubeObjects::Channel(it.value()));
                    else if (it.key() == "gridVideoRenderer" || it.key() == "videoRenderer")
                        UIUtils::addVideoToList(widget, InnertubeObjects::Video(it.value()));
                    QCoreApplication::processEvents();
                }
            }
        }
    }

    void setupLive(ContinuableListWidget* widget, const QJsonValue& renderer)
    {
        widget->toggleListGridLayout();
        if (qtTubeApp->settings().hideStreams)
        {
            widget->addItem("This tab is disabled because the live streams filter is turned on.");
            return;
        }

        const QJsonArray contents = renderer["content"]["richGridRenderer"]["contents"].toArray();
        for (const QJsonValue& v : contents)
        {
            if (const QJsonValue video = v["richItemRenderer"]["content"]["videoRenderer"]; video.isObject())
                UIUtils::addVideoToList(widget, InnertubeObjects::Video(video));
            else if (const QJsonValue continuation = v["continuationItemRenderer"]; continuation.isObject())
                widget->continuationToken = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            QCoreApplication::processEvents();
        }

        if (widget->count() == 0)
            widget->addItem("This channel has no live streams.");
    }

    void setupMembership(ContinuableListWidget* widget, const QJsonValue& renderer)
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

            TubeLabel* badgeLabel = new TubeLabel;
            badgeLabel->setFixedSize(18, 18);
            badgeLabel->setScaledContents(true);

            InnertubeObjects::ResponsiveImage badge(perks["badge"]["thumbnails"]);
            if (const InnertubeObjects::GenericThumbnail* recBadge = badge.recommendedQuality(badgeLabel->size()))
                badgeLabel->setImage(recBadge->url);

            TubeLabel* membershipTitle = new TubeLabel(InnertubeObjects::InnertubeString(perks["title"]).text);

            perksHeader->addWidget(badgeLabel);
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

                    TubeLabel* titleLabel = new TubeLabel(InnertubeObjects::InnertubeString(perkRenderer["title"]).text);
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

                            TubeLabel* loyaltyBadgeLabel = new TubeLabel(InnertubeObjects::InnertubeString(loyaltyBadge["title"]).text);
                            loyaltyBadgeLayout->addWidget(loyaltyBadgeLabel);

                            TubeLabel* loyaltyBadgeIcon = new TubeLabel;
                            loyaltyBadgeIcon->setFixedSize(18, 18);
                            loyaltyBadgeIcon->setScaledContents(true);

                            InnertubeObjects::ResponsiveImage icon(loyaltyBadge["icon"]["thumbnails"]);
                            if (const InnertubeObjects::GenericThumbnail* recIcon = icon.recommendedQuality(loyaltyBadgeIcon->size()))
                                loyaltyBadgeIcon->setImage(recIcon->url);

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
                            TubeLabel* thumbnailLabel = new TubeLabel;
                            thumbnailLabel->setFixedSize(32, 32);
                            thumbnailLabel->setScaledContents(true);

                            InnertubeObjects::ResponsiveImage thumbnail(v2["thumbnails"]);
                            if (const InnertubeObjects::GenericThumbnail* recThumb = thumbnail.recommendedQuality(thumbnailLabel->size()))
                                thumbnailLabel->setImage(recThumb->url);

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
            if (const QJsonValue videoRenderer = v["videoRenderer"]; videoRenderer.isObject())
                UIUtils::addVideoToList(widget, InnertubeObjects::Video(videoRenderer));
            else if (const QJsonValue continuation = v["continuationItemRenderer"]; continuation.isObject())
                widget->continuationToken = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            QCoreApplication::processEvents();
        }
    }

    void setupShorts(ContinuableListWidget* widget, const QJsonValue& renderer)
    {
        widget->toggleListGridLayout();
        if (qtTubeApp->settings().hideShorts)
        {
            widget->addItem("This tab is disabled because the shorts filter is turned on.");
            return;
        }

        const QJsonArray contents = renderer["content"]["richGridRenderer"]["contents"].toArray();
        for (const QJsonValue& v : contents)
        {
            if (const QJsonValue reel = v["richItemRenderer"]["content"]["reelItemRenderer"]; reel.isObject())
                UIUtils::addVideoToList(widget, InnertubeObjects::Reel(reel));
            else if (const QJsonValue sl = v["richItemRenderer"]["content"]["shortsLockupViewModel"]; sl.isObject())
                UIUtils::addVideoToList(widget, InnertubeObjects::ShortsLockupViewModel(sl));
            else if (const QJsonValue continuation = v["continuationItemRenderer"]; continuation.isObject())
                widget->continuationToken = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            QCoreApplication::processEvents();
        }

        if (widget->count() == 0)
            widget->addItem("This channel has no shorts.");
    }

    void setupUnimplemented(ContinuableListWidget* widget)
    {
        widget->addItem("This tab is unimplemented.");
    }

    void setupVideos(ContinuableListWidget* widget, const QJsonValue& renderer)
    {
        widget->toggleListGridLayout();
        // TODO: add filtering
        const QJsonArray contents = renderer["content"]["richGridRenderer"]["contents"].toArray();
        for (const QJsonValue& v : contents)
        {
            if (const QJsonValue video = v["richItemRenderer"]["content"]["videoRenderer"]; video.isObject())
                UIUtils::addVideoToList(widget, InnertubeObjects::Video(video));
            else if (const QJsonValue continuation = v["continuationItemRenderer"]; continuation.isObject())
                widget->continuationToken = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            QCoreApplication::processEvents();
        }

        if (widget->count() == 0)
            widget->addItem("This channel has no videos.");
    }
}
