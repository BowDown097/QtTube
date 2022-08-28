#ifndef BROWSEHELPER_HPP
#define BROWSEHELPER_HPP
#include <QListWidgetItem>
#include <QScrollBar>
#include <QUrl>
#include <QtConcurrent>
#include <QtNetwork>
#include <innertube.hpp>
#include <ui/homevideorenderer.h>

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
            QMessageBox::critical(nullptr, "Failed to get home browsing info", ie.message());
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
            QMessageBox::critical(nullptr, "Failed to get subscriptions browsing info", ie.message());
        }
    }

    template<typename T>
    typename std::enable_if_t<std::is_base_of_v<InnertubeEndpoints::BaseEndpoint, T>, void> tryContinuation(int value, QListWidget* widget)
    {
        if (value < widget->verticalScrollBar()->maximum() - 10 || continuationOngoing || InnerTube::instance().context()->client.visitorData.isEmpty())
            return;

        continuationOngoing = true;

        try
        {
            T newData = InnerTube::instance().get<T>("", continuationToken);
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
    void setupVideoList(const QVector<InnertubeObjects::Video>& videos, QListWidget* widget)
    {
        for (const InnertubeObjects::Video& video : videos)
        {
            HomeVideoRenderer* renderer = new HomeVideoRenderer;
            renderer->setChannelData(video.owner);
            renderer->setVideoData(video.isLive, video.lengthText.text, video.publishedTimeText.text, video.title.text, video.videoId, video.viewCountText.text);

            QListWidgetItem* item = new QListWidgetItem(widget);
            item->setSizeHint(renderer->sizeHint());
            widget->addItem(item);
            widget->setItemWidget(item, renderer);

            static_cast<void>(QtConcurrent::run([renderer, video] {
                QNetworkAccessManager manager;
                QNetworkRequest request(QUrl(video.thumbnail.mqdefault));
                QNetworkReply* reply = manager.get(request);

                QEventLoop loop;
                QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                loop.exec();
                renderer->setThumbnail(reply->readAll());
                reply->deleteLater();
            }));
        }
    }
};

#endif // BROWSEHELPER_HPP
