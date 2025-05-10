#pragma once
#include "qttube-plugin/plugininterface.h"

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

class YouTubeAuth : public QtTube::PluginAuthentication
{
public:
    void clear() override;
    void init() override;
    void save() override;
private:
    QList<CredentialSet> m_credentials;
};
