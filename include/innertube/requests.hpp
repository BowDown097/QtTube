#ifndef REQUESTS_HPP
#define REQUESTS_HPP
#include "objects/InnertubeContext.h"
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

class InnertubeRequests
{
public:
    static QString browse(InnertubeContext context, bool loggedIn = false, const QString& sapisid = "", bool useShelves = false)
    {
        if (useShelves)
        {
            context.client.clientName = "ANDROID";
            context.client.clientVersion = "15.14.33";
        }

        QNetworkAccessManager* manager = new QNetworkAccessManager;
        QNetworkRequest request(QUrl("https://www.youtube.com/youtubei/v1/browse?key=AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8"));

        if (loggedIn)
        {
            request.setRawHeader("Authorization", sapisid.toUtf8());
            request.setRawHeader("X-Goog-AuthUser", "0");
        }

        request.setRawHeader("X-Goog-Visitor-Id", context.client.visitorData.toLatin1());
        request.setRawHeader("X-YOUTUBE-CLIENT-NAME", context.client.clientName.toLatin1());
        request.setRawHeader("X-YOUTUBE-CLIENT-VERSION", context.client.clientVersion.toLatin1());
        request.setRawHeader("X-ORIGIN", "https://www.youtube.com");

        QJsonObject body = {
            { "context", context.toJson() },
            { "browseId", "FEwhat_to_watch" }
        };

        QNetworkReply* reply = manager->post(request, QJsonDocument(body).toJson());
        QEventLoop event;
        QObject::connect(reply, &QNetworkReply::finished, &event, &QEventLoop::quit);
        event.exec();

        return reply->readAll();
    }
};

#endif // REQUESTS_HPP
