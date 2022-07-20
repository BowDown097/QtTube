#ifndef INNERTUBECLIENT_H
#define INNERTUBECLIENT_H
#include <QJsonObject>
#include <QString>

class InnertubeConfigInfo
{
public:
    QString appInstallData;
    InnertubeConfigInfo(const QString& aid) : appInstallData(aid) {}
    QJsonObject toJson() const { return {{ "appInstallData", appInstallData }}; }
};

class InnertubeClient
{
public:
    QString browserName;
    QString browserVersion;
    QString clientFormFactor;
    QString clientName;
    QString clientVersion;
    InnertubeConfigInfo configInfo;
    QString deviceMake;
    QString deviceModel;
    QString gl;
    QString hl;
    QString originalUrl;
    QString osName;
    QString osVersion;
    QString platform;
    QString remoteHost;
    int screenDensityFloat;
    int screenPixelDensity;
    QString timeZone;
    QString userAgent;
    QString userInterfaceTheme;
    QString visitorData;

    InnertubeClient(const QString& bName, const QString& bVer, const QString& cFF, const QString& cName, const QString& cVer, const InnertubeConfigInfo& cI,
                    const QString& dMake, const QString& dModel, const QString& _gl, const QString& _hl, const QString& origUrl, const QString& oName,
                    const QString& oVer, const QString& plat, const QString& rHost, int sDF, int sPD, const QString& tz, const QString& agent,
                    const QString& theme, const QString& visitor)
        : browserName(bName), browserVersion(bVer), clientFormFactor(cFF), clientName(cName), clientVersion(cVer), configInfo(cI), deviceMake(dMake),
          deviceModel(dModel), gl(_gl), hl(_hl), originalUrl(origUrl), osName(oName), osVersion(oVer), platform(plat), remoteHost(rHost),
          screenDensityFloat(sDF), screenPixelDensity(sPD), timeZone(tz), userAgent(agent), userInterfaceTheme(theme), visitorData(visitor) {}

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
