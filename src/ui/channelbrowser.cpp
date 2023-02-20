#include "channelbrowser.h"
#include "innertube/innertubeexception.h"
#include "innertube/objects/channel/aboutfullmetadata.h"
#include "uiutilities.h"
#include "ui/widgets/tubelabel.h"
#include <QApplication>
#include <QDesktopServices>
#include <QJsonObject>
#include <QUrlQuery>

void ChannelBrowser::setupAbout(QListWidget* channelTab, const QJsonValue& tabRenderer)
{
    const QJsonObject metadataRenderer = tabRenderer["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]
            ["contents"][0]["channelAboutFullMetadataRenderer"].toObject();
    if (metadataRenderer.isEmpty())
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

            QObject::connect(label, &TubeLabel::clicked, [link]
            {
                QUrl url(link.navigationEndpoint["urlEndpoint"]["url"].toString());
                QUrlQuery query(url);
                QDesktopServices::openUrl(QUrl::fromPercentEncoding(query.queryItemValue("q").toUtf8()));
            });

            QListWidgetItem* item = new QListWidgetItem(channelTab);
            item->setSizeHint(label->sizeHint());
            channelTab->addItem(item);
            channelTab->setItemWidget(item, label);
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
            const QJsonObject o2 = v2.toObject();
            if (o2.contains("gridRenderer"))
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
            else if (o2.contains("shelfRenderer"))
            {
                UIUtilities::addShelfTitleToList(channelTab, v2["shelfRenderer"]);
                QJsonArray shelfItems = v2["shelfRenderer"]["content"]["horizontalListRenderer"]["items"].toArray();
                if (shelfItems.isEmpty()) // if no horizontal list, try expanded contents
                    shelfItems = v2["shelfRenderer"]["content"]["expandedShelfContentsRenderer"]["items"].toArray();

                for (const QJsonValue& v3 : qAsConst(shelfItems))
                {
                    const QJsonObject obj = v3.toObject();
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
            if (!v2.toObject().contains("shelfRenderer"))
                continue;

            UIUtilities::addShelfTitleToList(channelTab, v2["shelfRenderer"]);
            QJsonArray list = v2["shelfRenderer"]["content"]["horizontalListRenderer"]["items"].toArray();
            if (list.isEmpty()) // if no horizontal list, try expanded contents
                list = v2["shelfRenderer"]["content"]["expandedShelfContentsRenderer"]["items"].toArray();

            for (const QJsonValue& v3 : qAsConst(list))
            {
                const QJsonObject obj = v3.toObject();
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
        if (!v.toObject().contains("richItemRenderer"))
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

void ChannelBrowser::setupShorts(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp)
{
    const QJsonArray contents = tabRenderer["content"]["richGridRenderer"]["contents"].toArray();
    for (const QJsonValue& v : contents)
    {
        if (!v.toObject().contains("richItemRenderer"))
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
        if (!v.toObject().contains("richItemRenderer"))
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
