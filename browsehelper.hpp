#ifndef BROWSEHELPER_HPP
#define BROWSEHELPER_HPP
#include <QListWidgetItem>
#include <QUrl>
#include <QtConcurrent>
#include <QtNetwork>
#include <innertube.hpp>
#include <ui/homevideorenderer.h>

class BrowseHelper
{
public:
    static void browseHome(QListWidget* homeWidget)
    {
        try
        {
            QVector<InnertubeObjects::Video> videos = InnerTube::instance().get<InnertubeEndpoints::BrowseHome>().videos;
            QVector<HomeVideoRenderer*> renderers;
            for (const InnertubeObjects::Video& video : videos)
            {
                HomeVideoRenderer* renderer = new HomeVideoRenderer;
                renderer->setChannelData(video.owner);
                renderer->setVideoData(video.isLive, video.lengthText.text, video.publishedTimeText.text, video.title.text, video.videoId, video.viewCountText.text);

                QListWidgetItem* item = new QListWidgetItem(homeWidget);
                item->setSizeHint(renderer->sizeHint());
                homeWidget->addItem(item);
                homeWidget->setItemWidget(item, renderer);
                renderers.append(renderer);
            }

            static_cast<void>(QtConcurrent::run([renderers, videos] {
                QNetworkAccessManager manager;
                for (int i = 0; i < videos.length(); i++)
                {
                    HomeVideoRenderer* const& renderer = renderers[i];
                    const InnertubeObjects::Video& video = videos[i];

                    QNetworkRequest request(QUrl(video.thumbnail.mqdefault));
                    QNetworkReply* reply = manager.get(request);

                    QEventLoop loop;
                    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                    loop.exec();
                    renderer->setThumbnail(reply->readAll());
                }
            }));
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get home browsing info", ie.message());
        }
    }

    static void browseSubscriptions(QListWidget* subsWidget)
    {
        try
        {
            if (!InnerTube::instance().hasAuthenticated())
            {
                subsWidget->addItem("You need to log in to view subscriptions.");
                return;
            }

            QVector<InnertubeObjects::Video> videos = InnerTube::instance().get<InnertubeEndpoints::BrowseSubscriptions>().videos;
            QVector<HomeVideoRenderer*> renderers;
            for (const InnertubeObjects::Video& video : videos)
            {
                HomeVideoRenderer* renderer = new HomeVideoRenderer;
                renderer->setChannelData(video.owner);
                renderer->setVideoData(video.isLive, video.lengthText.text, video.publishedTimeText.text, video.title.text, video.videoId, video.viewCountText.text);

                QListWidgetItem* item = new QListWidgetItem(subsWidget);
                item->setSizeHint(renderer->sizeHint());
                subsWidget->addItem(item);
                subsWidget->setItemWidget(item, renderer);
                renderers.append(renderer);
            }

            static_cast<void>(QtConcurrent::run([renderers, videos] {
                QNetworkAccessManager manager;
                for (int i = 0; i < videos.length(); i++)
                {
                    HomeVideoRenderer* const& renderer = renderers[i];
                    const InnertubeObjects::Video& video = videos[i];

                    QNetworkRequest request(QUrl(video.thumbnail.mqdefault));
                    QNetworkReply* reply = manager.get(request);

                    QEventLoop loop;
                    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                    loop.exec();
                    renderer->setThumbnail(reply->readAll());
                }
            }));
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get subscriptions browsing info", ie.message());
        }
    }
};

#endif // BROWSEHELPER_HPP
