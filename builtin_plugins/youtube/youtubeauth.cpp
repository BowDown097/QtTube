#include "youtubeauth.h"
#include "innertube.h"
#include "protobuf/protobufcompiler.h"

const QtTubePlugin::AuthStore::AuthUser* YouTubeAuth::activeLogin() const
{
    auto it = std::ranges::find_if(m_credentials, &CredentialSet::active);
    return it != m_credentials.end() ? &(*it) : nullptr;
}

void YouTubeAuth::clear()
{
    QtTubePlugin::AuthStore::clear();
    m_credentials.clear();
}

void YouTubeAuth::init()
{
    QSettings settings(configPath(), QSettings::IniFormat);
    const QStringList childGroups = settings.childGroups();

    for (const QString& group : childGroups)
    {
        settings.beginGroup(group);
        m_credentials.append(CredentialSet(
            settings.value("active", false).toBool(), // active
            settings.value("avatar").toString(), // avatar
            group, // id
            settings.value("username").toString(), // username
            settings.value("apisid").toString(), // apisid
            settings.value("hsid").toString(), // hsid
            settings.value("sapisid").toString(), // sapisid
            settings.value("sid").toString(), // sid
            settings.value("ssid").toString(), // ssid
            settings.value("visitorInfo").toString() // visitorInfo
        ));
        settings.endGroup();
    }
}

void YouTubeAuth::populateAuthStore(const CredentialSet& credSet)
{
    InnertubeAuthStore* authStore = InnerTube::instance()->authStore();
    authStore->apisid = credSet.apisid;
    authStore->hsid = credSet.hsid;
    authStore->sapisid = credSet.sapisid;
    authStore->sid = credSet.sid;
    authStore->ssid = credSet.ssid;
    authStore->visitorInfo = credSet.visitorInfo;
    InnerTube::instance()->context()->client.visitorData = ProtobufCompiler::padded(credSet.visitorInfo);
}

void YouTubeAuth::save()
{
    if (m_credentials.empty())
        return;

    QSettings settings(configPath(), QSettings::IniFormat);
    for (const CredentialSet& credSet : std::as_const(m_credentials))
    {
        settings.beginGroup(credSet.id);
        settings.setValue("active", credSet.active);
        settings.setValue("apisid", credSet.apisid);
        settings.setValue("avatar", credSet.avatar);
        settings.setValue("hsid", credSet.hsid);
        settings.setValue("sapisid", credSet.sapisid);
        settings.setValue("sid", credSet.sid);
        settings.setValue("ssid", credSet.ssid);
        settings.setValue("username", credSet.username);
        settings.setValue("visitorInfo", credSet.visitorInfo);
        settings.endGroup();
    }
}
