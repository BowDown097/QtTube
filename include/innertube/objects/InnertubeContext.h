#ifndef INNERTUBECONTEXT_H
#define INNERTUBECONTEXT_H
#include "InnertubeClient.h"

class InnertubeClickTracking
{
public:
    QString clickTrackingParams;
    InnertubeClickTracking(const QString& ctp) : clickTrackingParams(ctp) {}
    QJsonObject toJson() const { return {{ "clickTrackingParams", clickTrackingParams }}; }
};

class InnertubeRequestConfig
{
public:
    bool useSsl;
    InnertubeRequestConfig(bool ssl) : useSsl(ssl) {}
    QJsonObject toJson() const { return {{ "useSsl", useSsl }}; }
};

class InnertubeUserConfig
{
public:
    bool lockedSafetyMode;
    InnertubeUserConfig(bool lsm) : lockedSafetyMode(lsm) {}
    QJsonObject toJson() const { return {{ "lockedSafetyMode", lockedSafetyMode }}; }
};

class InnertubeContext
{
public:
    InnertubeClickTracking clickTracking;
    InnertubeClient client;
    InnertubeRequestConfig request;
    InnertubeUserConfig user;

    InnertubeContext(const InnertubeClickTracking& ict, const InnertubeClient& ic, const InnertubeRequestConfig& irc, const InnertubeUserConfig& iuc)
        : clickTracking(ict), client(ic), request(irc), user(iuc) {}

    QJsonObject toJson() const
    {
        return {
            { "clickTracking", clickTracking.toJson() },
            { "client", client.toJson() },
            { "request", request.toJson() },
            { "user", user.toJson() }
        };
    }
};

#endif // INNERTUBECONTEXT_H
