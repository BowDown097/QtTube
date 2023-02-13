#include "channelbrowser.h"
#include "http.h"
#include "innertube/innertubeexception.h"
#include "innertube/objects/channel/aboutfullmetadata.h"
#include "settingsstore.h"
#include "ui/widgets/browsechannelrenderer.h"
#include "ui/widgets/browsevideorenderer.h"
#include "ui/widgets/tubelabel.h"
#include <QApplication>
#include <QComboBox>
#include <QDesktopServices>

void ChannelBrowser::setupAbout(QListWidget* channelTab, const QJsonValue& tabRenderer)
{
    const QJsonObject metadataRenderer = tabRenderer["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]
            ["contents"][0]["channelAboutFullMetadataRenderer"].toObject();
    if (metadataRenderer.isEmpty())
        throw InnertubeException("[SetupAbout] channelAboutFullMetadataRenderer not found");

    InnertubeObjects::AboutFullMetadata metadata(metadataRenderer);

    addBoldLabel(channelTab, metadata.viewCountText);
    addLabel(channelTab, metadata.joinedDateText.text);

    if (metadata.showDescription && !metadata.description.isEmpty())
    {
        addSeparatorItem(channelTab);
        addBoldLabel(channelTab, metadata.descriptionLabel.text);
        addLabel(channelTab, metadata.description);
    }

    if (!metadata.country.isEmpty())
    {
        addSeparatorItem(channelTab);
        addBoldLabel(channelTab, metadata.detailsLabel.text);
        addLabel(channelTab, metadata.countryLabel.text.trimmed() + " " + metadata.country);
    }

    if (!metadata.primaryLinks.isEmpty())
    {
        addSeparatorItem(channelTab);
        addBoldLabel(channelTab, metadata.primaryLinksLabel.text);

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
            if (v2.toObject().contains("messageRenderer"))
            {
                QListWidgetItem* item = new QListWidgetItem("This channel doesn't feature any other channels.", channelTab);
                channelTab->addItem(item);
                return;
            }

            const QJsonArray gridItems = v2["gridRenderer"]["items"].toArray();
            for (const QJsonValue& v3 : gridItems)
            {
                if (!v3.toObject().contains("gridChannelRenderer"))
                    continue;

                InnertubeObjects::Channel channel(v3["gridChannelRenderer"]);
                addChannelRendererFromChannel(channelTab, channel);
            }
        }
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

            QJsonValue shelf = v2["shelfRenderer"];
            TubeLabel* shelfLabel = new TubeLabel(InnertubeObjects::InnertubeString(shelf["title"]).text);
            shelfLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));

            QListWidgetItem* item = new QListWidgetItem(channelTab);
            item->setSizeHint(shelfLabel->sizeHint());
            channelTab->addItem(item);
            channelTab->setItemWidget(item, shelfLabel);

            QJsonArray list = shelf["content"]["horizontalListRenderer"]["items"].toArray();
            if (list.isEmpty()) // if no horizontal list, try expanded contents
                list = shelf["content"]["expandedShelfContentsRenderer"]["items"].toArray();

            for (const QJsonValue& v3 : qAsConst(list))
            {
                const QJsonObject obj = v3.toObject();
                QJsonObject::const_iterator it = obj.begin();
                if (it.key() == "channelRenderer" || it.key() == "gridChannelRenderer")
                {
                    InnertubeObjects::Channel channel(it.value());
                    addChannelRendererFromChannel(channelTab, channel);
                }
                else if (it.key() == "gridVideoRenderer" || it.key() == "videoRenderer")
                {
                    // id and name are missing a lot of the time, so we need to populate them manually
                    InnertubeObjects::Video video(it.value(), it.key() == "gridVideoRenderer");
                    video.owner.id = channelResp.metadata.externalId;
                    video.owner.name = channelResp.metadata.title;
                    addVideoRendererFromVideo(channelTab, video);
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
        addVideoRendererFromVideo(channelTab, video);
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
        addVideoRendererFromReel(channelTab, reel);
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
        addVideoRendererFromVideo(channelTab, video);
    }
}

void ChannelBrowser::addChannelRendererFromChannel(QListWidget* list, const InnertubeObjects::Channel& channel)
{
    BrowseChannelRenderer* renderer = new BrowseChannelRenderer;
    renderer->setData(channel.channelId, channel.descriptionSnippet.text, channel.title.text, channel.subscribed,
                      channel.subscriberCountText.text, channel.videoCountText.text);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(renderer->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, renderer);

    HttpReply* reply = Http::instance().get(channel.thumbnails.last().url);
    QObject::connect(reply, &HttpReply::finished, renderer, &BrowseChannelRenderer::setThumbnail);
}

void ChannelBrowser::addVideoRendererFromReel(QListWidget* list, const InnertubeObjects::Reel& reel)
{
    BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
    renderer->setChannelData(reel.owner);
    renderer->setVideoData("SHORTS", "", 0, reel.headline, reel.videoId, reel.viewCountText.text);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(renderer->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, renderer);

    HttpReply* reply = Http::instance().get(reel.thumbnails[0].url);
    QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
}

void ChannelBrowser::addVideoRendererFromVideo(QListWidget* list, const InnertubeObjects::Video& video)
{
    BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
    renderer->setChannelData(video.owner);
    renderer->setVideoData(video.lengthText.text, video.publishedTimeText.text, video.startTimeSeconds, video.title.text,
        video.videoId, SettingsStore::instance().condensedViews ? video.shortViewCountText.text : video.viewCountText.text);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(renderer->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, renderer);

    HttpReply* reply = Http::instance().get(video.thumbnail.mqdefault);
    QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
}

void ChannelBrowser::addBoldLabel(QListWidget* list, const QString& text)
{
    QLabel* label = new QLabel(text);
    label->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(label->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, label);
}

void ChannelBrowser::addLabel(QListWidget* list, const QString& text)
{
    QLabel* label = new QLabel(text);
    label->setWordWrap(true);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(label->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, label);
}

void ChannelBrowser::addSeparatorItem(QListWidget* list)
{
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(line->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, line);
}
