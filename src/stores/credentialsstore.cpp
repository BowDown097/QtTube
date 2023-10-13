#include "credentialsstore.h"
#include "innertube.h"
#include "protobuf/simpleprotobuf.h"
#include <QSettings>

CredentialsStore* CredentialsStore::instance()
{
    std::call_once(m_onceFlag, [] { m_instance = new CredentialsStore; });
    return m_instance;
}

qsizetype CredentialsStore::getActiveLoginIndex() const
{
    auto it = std::ranges::find_if(credentials, [](const CredentialSet& credSet) { return credSet.active; });
    qsizetype index = std::distance(credentials.begin(), it);
    return index != credentials.size() ? index : -1;
}

void CredentialsStore::initializeFromStoreFile()
{
    QSettings settings(configPath, QSettings::IniFormat);

    for (const QString& group : settings.childGroups())
    {
        settings.beginGroup(group);
        credentials.append(CredentialSet
        {
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

void CredentialsStore::populateAuthStore(int index)
{
    if (index < 0 || index >= credentials.size())
        return;

    const CredentialSet& credSet = credentials[index];
    InnertubeAuthStore* authStore = InnerTube::instance().authStore();

    authStore->apisid = credSet.apisid;
    authStore->hsid = credSet.hsid;
    authStore->sapisid = credSet.sapisid;
    authStore->sid = credSet.sid;
    authStore->ssid = credSet.ssid;
    authStore->visitorInfo = credSet.visitorInfo;
    InnerTube::instance().context()->client.visitorData = SimpleProtobuf::padded(credSet.visitorInfo);
}

void CredentialsStore::saveToStoreFile()
{
    QSettings settings(configPath, QSettings::IniFormat);

    for (const CredentialSet& credSet : credentials)
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

void CredentialsStore::updateAccount(const InnertubeEndpoints::AccountMenu& accountMenuData)
{
    const QList<InnertubeObjects::CompactLink>& section = accountMenuData.response.sections[0];
    auto iter = std::ranges::find_if(section, [](const InnertubeObjects::CompactLink& link) { return link.iconType == "ACCOUNT_BOX"; });
    if (iter == section.end())
        return;

    QString avatarUrl = accountMenuData.response.header.accountPhotos[0].url;
    QString channelId = iter->navigationEndpoint["browseEndpoint"]["browseId"].toString();
    QString username = accountMenuData.response.header.accountName;

    qsizetype activeIndex = getActiveLoginIndex();
    if (activeIndex != -1)
    {
        CredentialSet& credSet = credentials[activeIndex];
        if (credSet.channelId != channelId)
        {
            credSet.active = false;
            auto credIter = std::ranges::find_if(credentials, [&channelId](const CredentialSet& credSet) {
                return credSet.channelId == channelId;
            });
            if (credIter != credentials.end())
            {
                credIter->active = true;
                credIter->avatarUrl = avatarUrl;
                credIter->username = username;
                saveToStoreFile();
                return;
            }
        }
        else
        {
            credSet.avatarUrl = avatarUrl;
            credSet.username = username;
            saveToStoreFile();
            return;
        }
    }

    const InnertubeAuthStore* authStore = InnerTube::instance().authStore();
    credentials.append(CredentialSet {
        .active = true,
        .apisid = authStore->apisid,
        .avatarUrl = avatarUrl,
        .channelId = channelId,
        .hsid = authStore->hsid,
        .sapisid = authStore->sapisid,
        .sid = authStore->sid,
        .ssid = authStore->ssid,
        .username = username,
        .visitorInfo = authStore->visitorInfo
    });
    saveToStoreFile();
}
