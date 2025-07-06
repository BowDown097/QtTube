#pragma once
#include "qttube-plugin/components/authstore.h"

struct CredentialSet : QtTubePlugin::AuthStore::AuthUser
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
        bool active, const QString& avatar, const QString& id, const QString& username,
        const QString& apisid, const QString& hsid, const QString& sapisid,
        const QString& sid, const QString& ssid, const QString& visitorInfo)
        : QtTubePlugin::AuthStore::AuthUser(active, avatar, id, username),
          apisid(apisid), hsid(hsid), sapisid(sapisid),
          sid(sid), ssid(ssid), visitorInfo(visitorInfo) {}
};

class YouTubeAuth : public QtTubePlugin::AuthStore
{
public:
    const QtTubePlugin::AuthStore::AuthUser* activeLogin() const override;
    void clear() override;
    void init() override;
    void save() override;

    void populateAuthStore(const CredentialSet& credSet);
private:
    QList<CredentialSet> m_credentials;
};
