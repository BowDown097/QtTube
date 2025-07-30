#pragma once
#include "qttube-plugin/components/auth/authstore.h"
#include "qttube-plugin/components/auth/webauthroutine.h"

struct CredentialSet : QtTubePlugin::AuthUser
{
    QString apisid;
    QString hsid;
    QString sapisid;
    QString sid;
    QString ssid;
    QString visitorInfo;

    friend bool operator==(const CredentialSet& lhs, const CredentialSet& rhs)
    { return lhs.id == rhs.id; }

    CredentialSet() = default;
    CredentialSet(
        bool active, const QString& avatar, const QString& id, const QString& username, const QString& handle,
        const QString& apisid, const QString& hsid, const QString& sapisid,
        const QString& sid, const QString& ssid, const QString& visitorInfo)
        : QtTubePlugin::AuthUser(active, avatar, id, username, handle),
          apisid(apisid), hsid(hsid), sapisid(sapisid),
          sid(sid), ssid(ssid), visitorInfo(visitorInfo) {}
};

struct YouTubeAuthRoutine : QtTubePlugin::WebAuthRoutine
{
    void onNewCookie(const QByteArray& name, const QByteArray& value) override;
    void onNewHeader(const QByteArray& name, const QByteArray& value) override;
    void start() override;
};

struct YouTubeAuth : QtTubePlugin::AuthStore<CredentialSet, YouTubeAuthRoutine>
{
    void init() override;
    void save() override;

    void restoreFromActive() override;
    void unauthenticate() override;

    CredentialSet createUser(const QtTubePlugin::InitialAccountData& data, const YouTubeAuthRoutine* routine) override;

    void populateAuthStore(const CredentialSet& credSet);
};
