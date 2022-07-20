#ifndef INNERTUBECLIENT_H
#define INNERTUBECLIENT_H
#include "InnertubeConfigInfo.h"

class InnertubeClient
{
public:
    QString browserName, browserVersion, clientFormFactor, clientName, clientVersion, deviceMake, deviceModel, gl, hl, originalUrl, osName, osVersion,
        platform, remoteHost, timeZone, userAgent, userInterfaceTheme, visitorData;
    InnertubeConfigInfo configInfo;
    int screenDensityFloat, screenPixelDensity;

    InnertubeClient() {}
    InnertubeClient(const QString& cliName, const QString& cliVer, const QString& plat, const QString& theme, const QString& bName = "Firefox",
                    const QString& bVer = "103.0", const QString& cFF = "UNKNOWN_FORM_FACTOR", const InnertubeConfigInfo& cI = InnertubeConfigInfo(),
                    const QString& dMake = "", const QString& dModel = "", const QString& _gl = "US", const QString& _hl = "en", const QString& origUrl = "",
                    const QString& _osName = "", const QString& osVer = "", const QString& rHost = "", int sDF = 2, int sPD = 2, const QString& tz = "",
                    const QString& agent = "", const QString& visitor = "")
        : browserName(bName), browserVersion(bVer), clientFormFactor(cFF), clientName(cliName), clientVersion(cliVer), deviceMake(dMake),
          deviceModel(dModel), gl(_gl), hl(_hl), originalUrl(origUrl), osName(_osName), osVersion(osVer), platform(plat), remoteHost(rHost), timeZone(tz),
          userAgent(agent), userInterfaceTheme(theme), visitorData(visitor), configInfo(cI), screenDensityFloat(sDF), screenPixelDensity(sPD) {}

    QJsonObject toJson() const
    {
        return {
            { "browserName", browserName },
            { "browserVersion", browserVersion },
            { "clientFormFactor", clientFormFactor },
            { "clientName", clientName },
            { "clientVersion", clientVersion },
            { "configInfo", configInfo.toJson() },
            { "deviceMake", deviceMake },
            { "deviceModel", deviceModel },
            { "gl", gl },
            { "hl", hl },
            { "originalUrl", originalUrl },
            { "osName", osName },
            { "osVersion", osVersion },
            { "platform", platform },
            { "remoteHost", remoteHost },
            { "screenDensityFloat", screenDensityFloat },
            { "screenPixelDensity", screenPixelDensity },
            { "timeZone", timeZone },
            { "userAgent", userAgent },
            { "userInterfaceTheme", userInterfaceTheme },
            { "visitorData", visitorData }
        };
    }
};

#endif // INNERTUBECLIENT_H
