#ifndef INNERTUBECONTEXT_H
#define INNERTUBECONTEXT_H
#include "InnertubeClickTracking.h"
#include "InnertubeClient.h"
#include "InnertubeRequestConfig.h"
#include "InnertubeUserConfig.h"

class InnertubeContext
{
public:
    InnertubeClickTracking clickTracking;
    InnertubeClient client;
    InnertubeRequestConfig request;
    InnertubeUserConfig user;

    InnertubeContext() {}
    InnertubeContext(const InnertubeClient& ic, const InnertubeClickTracking& ict = InnertubeClickTracking(),
                     const InnertubeRequestConfig& irc = InnertubeRequestConfig(), const InnertubeUserConfig& iuc = InnertubeUserConfig())
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
