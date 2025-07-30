#include "youtubeauth.h"
#include "innertube.h"
#include "protobuf/protobufcompiler.h"

void YouTubeAuthRoutine::onNewCookie(const QByteArray& name, const QByteArray& value)
{
    InnertubeAuthStore* authStore = InnerTube::instance()->authStore();
    if (name == "APISID")
        authStore->apisid = value;
    else if (name == "HSID")
        authStore->hsid = value;
    else if (name == "SAPISID")
        authStore->sapisid = value;
    else if (name == "SID")
        authStore->sid = value;
    else if (name == "SSID")
        authStore->ssid = value;
}

void YouTubeAuthRoutine::onNewHeader(const QByteArray& name, const QByteArray& value)
{
    // should only be getting visitor id
    InnerTube::instance()->context()->client.visitorData = InnerTube::instance()->authStore()->visitorInfo = value;
}

void YouTubeAuthRoutine::start()
{
    setSearchCookies({
        {.name = "APISID", .domain = ".youtube.com"},
        {.name = "HSID", .domain = ".youtube.com"},
        {.name = "SAPISID", .domain = ".youtube.com"},
        {.name = "SID", .domain = ".youtube.com"},
        {.name = "SSID", .domain = ".youtube.com"}
    });
    setSearchHeaders({"X-Goog-Visitor-Id"});
    setUrl(QUrl("https://accounts.google.com/ServiceLogin/signinchooser?service=youtube&uilel=3&passive=true&continue=https%3A%2F%2Fwww.youtube.com%2Fsignin%3Faction_handle_signin%3Dtrue%26app%3Ddesktop%26hl%3Den%26next%3Dhttps%253A%252F%252Fwww.youtube.com%252F&hl=en&ec=65620&flowName=GlifWebSignIn&flowEntry=ServiceLogin"));
    QtTubePlugin::WebAuthRoutine::start();
}

CredentialSet YouTubeAuth::createUser(const QtTubePlugin::InitialAccountData& data, const YouTubeAuthRoutine* routine)
{
    QHash<QByteArray, QByteArray> cookies = routine->searchCookies();
    const QHash<QByteArray, QByteArray>& headers = routine->searchHeaders();

    return CredentialSet(
        true,
        data.avatarUrl,
        data.channelId,
        data.displayName,
        data.handle,
        cookies["APISID"],
        cookies["HSID"],
        cookies["SAPISID"],
        cookies["SID"],
        cookies["SSID"],
        headers["X-Goog-Visitor-Id"]
    );
}

void YouTubeAuth::init()
{
    QSettings settings(configPath(), QSettings::IniFormat);
    const QStringList childGroups = settings.childGroups();

    for (const QString& group : childGroups)
    {
        settings.beginGroup(group);
        append(CredentialSet(
            settings.value("active", false).toBool(),
            settings.value("avatar").toString(),
            group,
            settings.value("username").toString(),
            settings.value("handle").toString(),
            settings.value("apisid").toString(),
            settings.value("hsid").toString(),
            settings.value("sapisid").toString(),
            settings.value("sid").toString(),
            settings.value("ssid").toString(),
            settings.value("visitorInfo").toString()
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

void YouTubeAuth::restoreFromActive()
{
    if (CredentialSet* active = activeLogin())
        populateAuthStore(*active);
}

void YouTubeAuth::save()
{
    if (isEmpty())
        return;

    QSettings settings(configPath(), QSettings::IniFormat);
    for (const CredentialSet* credSet : credentials())
    {
        settings.beginGroup(credSet->id);
        settings.setValue("active", credSet->active);
        settings.setValue("apisid", credSet->apisid);
        settings.setValue("avatar", credSet->avatar);
        settings.setValue("handle", credSet->handle);
        settings.setValue("hsid", credSet->hsid);
        settings.setValue("sapisid", credSet->sapisid);
        settings.setValue("sid", credSet->sid);
        settings.setValue("ssid", credSet->ssid);
        settings.setValue("username", credSet->username);
        settings.setValue("visitorInfo", credSet->visitorInfo);
        settings.endGroup();
    }
}

void YouTubeAuth::unauthenticate()
{
    InnerTube::instance()->unauthenticate();
}
