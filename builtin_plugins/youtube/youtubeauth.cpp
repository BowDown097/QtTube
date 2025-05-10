#include "youtubeauth.h"

void YouTubeAuth::clear()
{
    QtTube::PluginAuthentication::clear();
    m_credentials.clear();
}

void YouTubeAuth::init()
{
    QSettings settings(configPath(), QSettings::IniFormat);
    const QStringList childGroups = settings.childGroups();

    for (const QString& group : childGroups)
    {
        settings.beginGroup(group);
        m_credentials.append(CredentialSet {
            .active = settings.value("active", false).toBool(),
            .apisid = settings.value("apisid").toString(),
            .avatarUrl = settings.value("avatarUrl").toString(),
            .channelId = group,
            .hsid = settings.value("hsid").toString(),
            .sapisid = settings.value("sapisid").toString(),
            .sid = settings.value("sid").toString(),
            .ssid = settings.value("ssid").toString(),
            .username = settings.value("username").toString(),
            .visitorInfo = settings.value("visitorInfo").toString()
        });
        settings.endGroup();
    }
}

void YouTubeAuth::save()
{
    if (m_credentials.empty())
        return;

    QSettings settings(configPath(), QSettings::IniFormat);
    for (const CredentialSet& credSet : std::as_const(m_credentials))
    {
        settings.beginGroup(credSet.channelId);
        settings.setValue("active", credSet.active);
        settings.setValue("apisid", credSet.apisid);
        settings.setValue("avatarUrl", credSet.avatarUrl);
        settings.setValue("hsid", credSet.hsid);
        settings.setValue("sapisid", credSet.sapisid);
        settings.setValue("sid", credSet.sid);
        settings.setValue("ssid", credSet.ssid);
        settings.setValue("username", credSet.username);
        settings.setValue("visitorInfo", credSet.visitorInfo);
        settings.endGroup();
    }
}
