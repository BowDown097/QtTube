#ifndef CREDENTIALSSTORE_H
#define CREDENTIALSSTORE_H
#include "innertube/endpoints/misc/accountmenu.h"
#include <QDir>
#include <QStandardPaths>

struct CredentialSet
{
    bool active = false;
    QString apisid;
    QString avatarUrl;
    QString channelId;
    QString hsid;
    QString sapisid;
    QString sid;
    QString ssid;
    QString username;
    QString visitorInfo;

    friend bool operator==(const CredentialSet& lhs, const CredentialSet& rhs) { return lhs.channelId == rhs.channelId; }
};

class CredentialsStore : public QObject
{
    Q_OBJECT
public:
    // QStandardPaths::AppConfigLocation appears to not work in a static context, so we have to make it ourselves :(
    static inline const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                                           + QDir::separator() + "QtTube"
                                           + QDir::separator() + "store.ini";

    explicit CredentialsStore(QObject* parent = nullptr) : QObject(parent) {}

    CredentialSet activeLogin() const;
    QList<CredentialSet> credentials() const { return m_credentials; }

    void initialize();
    void save();

    void populateAuthStore(const CredentialSet& credSet);
    void updateAccount(const InnertubeEndpoints::AccountMenu& data);
private:
    QList<CredentialSet> m_credentials;
};

#endif // CREDENTIALSSTORE_H
