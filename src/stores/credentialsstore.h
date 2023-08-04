#ifndef CREDENTIALSSTORE_H
#define CREDENTIALSSTORE_H
#include "innertube/endpoints/misc/accountmenu.h"
#include <mutex>
#include <QDir>
#include <QStandardPaths>

struct CredentialSet
{
    bool active;
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
    static inline const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "QtTube"
                                             + QDir::separator() + "store.ini";

    QList<CredentialSet> credentials;

    static CredentialsStore* instance();
    explicit CredentialsStore(QObject* parent = nullptr) : QObject(parent) {}

    qsizetype getActiveLoginIndex() const;
    void initializeFromStoreFile();
    void populateAuthStore(int index);
    void saveToStoreFile();
    void updateAccount(const InnertubeEndpoints::AccountMenu& accountMenuData);
private:
    static inline CredentialsStore* m_instance;
    static inline std::once_flag m_onceFlag;
};

#endif // CREDENTIALSSTORE_H
