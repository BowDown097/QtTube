#ifndef BROWSEHELPER_HPP
#define BROWSEHELPER_HPP
#include "http.h"
#include "innertube.hpp"
#include "ui/browsechannelrenderer.h"
#include "ui/browsevideorenderer.h"
#include <QListWidgetItem>
#include <QScrollBar>

class BrowseHelper
{
    bool continuationOngoing;
    QString continuationToken;
public:
    static BrowseHelper& instance()
    {
        static BrowseHelper bh;
        return bh;
    }

    void browseHistory(QListWidget* historyWidget)
    {
        try
        {
            if (InnerTube::instance().hasAuthenticated())
            {
                InnertubeEndpoints::BrowseHistory historyData = InnerTube::instance().get<InnertubeEndpoints::BrowseHistory>();
                setupVideoList(historyData.videos, historyWidget);
                continuationToken = historyData.continuationToken;
            }
            else
            {
                historyWidget->addItem("Local history has not been implemented yet. You will need to log in.");
            }
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get history browsing data", ie.message());
        }
    }

    void browseHome(QListWidget* homeWidget)
    {
        try
        {
            InnertubeEndpoints::BrowseHome homeData = InnerTube::instance().get<InnertubeEndpoints::BrowseHome>();
            setupVideoList(homeData.videos, homeWidget);
            continuationToken = homeData.continuationToken;
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get home browsing data", ie.message());
        }
    }

    void browseSubscriptions(QListWidget* subsWidget)
    {
        try
        {
            if (!InnerTube::instance().hasAuthenticated())
            {
                subsWidget->addItem("You need to log in to view subscriptions.");
                return;
            }

            InnertubeEndpoints::BrowseSubscriptions subsData = InnerTube::instance().get<InnertubeEndpoints::BrowseSubscriptions>();
            setupVideoList(subsData.videos, subsWidget);
            continuationToken = subsData.continuationToken;
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get subscriptions browsing data", ie.message());
        }
    }

    void search(QListWidget* searchWidget, const QString& query)
    {
        try
        {
            InnertubeEndpoints::Search searchData = InnerTube::instance().get<InnertubeEndpoints::Search>(query);
            searchWidget->addItem(QStringLiteral("About %1 results").arg(QLocale::system().toString(searchData.estimatedResults)));
            setupChannelList(searchData.channels, searchWidget);
            setupVideoList(searchData.videos, searchWidget);
            continuationToken = searchData.continuationToken;
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get search data", ie.message());
        }
    }

    template<typename T>
    typename std::enable_if_t<std::is_base_of_v<InnertubeEndpoints::BaseEndpoint, T>, void> tryContinuation(int value, QListWidget* widget, const QString& data = "")
    {
        if (value < widget->verticalScrollBar()->maximum() - 10 || continuationOngoing || InnerTube::instance().context()->client.visitorData.isEmpty())
            return;

        continuationOngoing = true;

        try
        {
            T newData = InnerTube::instance().get<T>(data, continuationToken);
            if constexpr (std::is_same_v<T, InnertubeEndpoints::Search>)
                setupChannelList(newData.channels, widget);
            setupVideoList(newData.videos, widget);
            continuationToken = newData.continuationToken;
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get continuation browsing info", ie.message());
        }

        continuationOngoing = false;
    }
private:
    void setupChannelList(const QVector<InnertubeObjects::Channel>& channels, QListWidget* widget)
    {
        for (const InnertubeObjects::Channel& channel : channels)
        {
            BrowseChannelRenderer* renderer = new BrowseChannelRenderer;
            renderer->setData(channel.channelId, channel.descriptionSnippet.text, channel.title.text, channel.subscribed, channel.subscriberCountText.text,
                              channel.videoCountText.text);

            QListWidgetItem* item = new QListWidgetItem(widget);
            item->setSizeHint(renderer->sizeHint());
            widget->addItem(item);
            widget->setItemWidget(item, renderer);

            HttpReply* reply = Http::instance().get(channel.thumbnails.last().url);
            QObject::connect(reply, &HttpReply::finished, renderer, &BrowseChannelRenderer::setThumbnail);
        }
    }

    void setupVideoList(const QVector<InnertubeObjects::Video>& videos, QListWidget* widget)
    {
        for (const InnertubeObjects::Video& video : videos)
        {
            BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
            renderer->setChannelData(video.owner);
            renderer->setVideoData(video.lengthText.text, video.publishedTimeText.text, video.startTimeSeconds, video.title.text, video.videoId,
                                   video.viewCountText.text);

            QListWidgetItem* item = new QListWidgetItem(widget);
            item->setSizeHint(renderer->sizeHint());
            widget->addItem(item);
            widget->setItemWidget(item, renderer);

            HttpReply* reply = Http::instance().get(video.thumbnail.mqdefault);
            QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
        }
    }
};

#endif // BROWSEHELPER_HPP
