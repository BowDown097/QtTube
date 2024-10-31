#pragma once
#include "genericstore.h"
#include <QObject>

struct CredentialSet
{
    bool active{};
    QString apisid;
    QString avatarUrl;
    QString channelId;
    QString hsid;
    QString sapisid;
    QString sid;
    QString ssid;
    QString username;
    QString visitorInfo;

    friend bool operator==(const CredentialSet& lhs, const CredentialSet& rhs)
    { return lhs.channelId == rhs.channelId; }
};

namespace InnertubeEndpoints { class AccountMenu; }

class CredentialsStore : public GenericStore
{
    Q_OBJECT
public:
    explicit CredentialsStore(QObject* parent = nullptr) : GenericStore("store.ini") {}

    const CredentialSet* activeLogin() const;
    const QList<CredentialSet>& credentials() const { return m_credentials; }

    void initialize() override;
    void save() override;

    void populateAuthStore(const CredentialSet& credSet);
    void updateAccount(const InnertubeEndpoints::AccountMenu& data);
private:
    QList<CredentialSet> m_credentials;
};
