#ifndef REQUESTS_HPP
#define REQUESTS_HPP
#include "auth.hpp"
#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrlQuery>

class InnertubeRequests
{
public:
    static QString accountMenu(InnertubeContext context, const InnertubeAuthStore& authStore)
    {
        QNetworkAccessManager* manager = new QNetworkAccessManager;
        QNetworkRequest request(QUrl("https://www.youtube.com/youtubei/v1/account/account_menu?key=AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8"));
        request.setRawHeader("Authorization", authStore.generateSAPISIDHash().toUtf8());
        request.setRawHeader("Cookie", authStore.getNecessaryLoginCookies().toUtf8());
        request.setRawHeader("X-Goog-AuthUser", "0");
        request.setRawHeader("X-Goog-Visitor-Id", context.client.visitorData.toLatin1());
        request.setRawHeader("X-YOUTUBE-CLIENT-NAME", context.client.clientName.toLatin1());
        request.setRawHeader("X-YOUTUBE-CLIENT-VERSION", context.client.clientVersion.toLatin1());
        request.setRawHeader("X-ORIGIN", "https://www.youtube.com");

        QJsonObject body = {
            { "context", context.toJson() },
            { "deviceTheme", "DEVICE_THEME_SUPPORTED" },
            { "userInterfaceTheme", context.client.userInterfaceTheme }
        };

        QNetworkReply* reply = manager->post(request, QJsonDocument(body).toJson());
        QEventLoop event;
        QObject::connect(reply, &QNetworkReply::finished, &event, &QEventLoop::quit);
        event.exec();

        return reply->readAll();
    }

    static QString browse(InnertubeContext context, const InnertubeAuthStore& authStore = InnertubeAuthStore(), bool useShelves = false)
    {
        if (useShelves)
        {
            context.client.clientName = "ANDROID";
            context.client.clientVersion = "15.14.33";
        }

        QNetworkAccessManager* manager = new QNetworkAccessManager;
        QNetworkRequest request(QUrl("https://www.youtube.com/youtubei/v1/browse?key=AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8"));

        if (authStore.populated)
        {
            request.setRawHeader("Authorization", authStore.generateSAPISIDHash().toUtf8());
            request.setRawHeader("Cookie", authStore.getNecessaryLoginCookies().toUtf8());
            request.setRawHeader("X-Goog-AuthUser", "0");
        }

        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain;charset=UTF-8");
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
