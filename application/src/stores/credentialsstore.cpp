#include "credentialsstore.h"
#include "innertube.h"
#include "mainwindow.h"
#include "protobuf/protobufcompiler.h"
#include "utils/tubeutils.h"
#include <QMessageBox>
#include <QSettings>

const CredentialSet* CredentialsStore::activeLogin() const
{
    auto it = std::ranges::find_if(m_credentials, &CredentialSet::active);
    return it != m_credentials.end() ? &(*it) : nullptr;
}

void CredentialsStore::clear()
{
    GenericStore::clear();
    m_credentials.clear();
}

void CredentialsStore::initialize()
{
    QSettings settings(configPath(), QSettings::IniFormat);
    const QStringList childGroups = settings.childGroups();

    for (const QString& group : childGroups)
    {
        settings.beginGroup(group);
        m_credentials.append(CredentialSet
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

void CredentialsStore::populateAuthStore(const CredentialSet& credSet)
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

void CredentialsStore::save()
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

void CredentialsStore::updateAccount(const InnertubeEndpoints::AccountMenu& data)
{
    const QString& channelHandle = data.response.header.channelHandle;
    QString channelId;

    if (channelHandle.isEmpty() ||
        !(channelId = TubeUtils::getUcidFromUrl("https://www.youtube.com/" + channelHandle)).startsWith("UC"))
    {
        QMessageBox::critical(nullptr, "Invalid Login Credentials", "Your login credentials are invalid. They may have expired. You will be logged out, then try logging in again.");
        MainWindow::topbar()->signOut();
        return;
    }

    const InnertubeObjects::GenericThumbnail* bestPhoto = data.response.header.accountPhoto.bestQuality();
    QString avatarUrl = bestPhoto ? bestPhoto->url : QString();
    QString username = data.response.header.accountName;

    if (auto active = std::ranges::find_if(m_credentials, &CredentialSet::active); active != m_credentials.end())
    {
        if (active->channelId != channelId)
        {
            active->active = false;
            if (auto match = std::ranges::find(m_credentials, channelId, &CredentialSet::channelId); match != m_credentials.end())
            {
                match->active = true;
                match->avatarUrl = avatarUrl;
                match->username = username;
                save();
                return;
            }
        }
        else
        {
            active->avatarUrl = avatarUrl;
            active->username = username;
            save();
            return;
        }
    }

    const InnertubeAuthStore* authStore = InnerTube::instance()->authStore();
    m_credentials.append(CredentialSet {
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
    save();
}
