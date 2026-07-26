#pragma once
#include <QException>
#include <QFileInfo>
#include <qttube-plugin/pluginfwd.h>
#include <qttube-plugin/plugininterface.h>

class PluginLoadException : public QException
{
public:
    explicit PluginLoadException(const QString& message) : m_message(message) {}
    PluginLoadException* clone() const override { return new PluginLoadException(*this); }
    const QString& message() const { return m_message; }
    void raise() const override { throw *this; }
private:
    QString m_message;
};

class PluginEntry
{
public:
    struct Providers
    {
        QtTubePlugin::AuthenticationProvider* auth{};
        QtTubePlugin::ChannelProvider* channel{};
        QtTubePlugin::ChannelSubscriptionProvider* channelSub{};
        QtTubePlugin::HistoryProvider* history{};
        QtTubePlugin::HomeProvider* home{};
        QtTubePlugin::LiveChatProvider* liveChat{};
        QtTubePlugin::NotificationsProvider* notifs{};
        QtTubePlugin::SearchProvider* search{};
        QtTubePlugin::SettingsProvider* settings{};
        QtTubePlugin::SubFeedProvider* subFeed{};
        QtTubePlugin::TrendingProvider* trending{};
        QtTubePlugin::WatchProvider* watch{};
    };

    bool active{};
    QtTubePlugin::AuthStoreBase* authStore{};
    QFileInfo fileInfo;
    std::unique_ptr<QtTubePlugin::PluginInterface> interface;
    QtTubePlugin::PluginMetadata metadata;
    Providers providers;
    QtTubePlugin::SettingsStore* settingsStore{};

    virtual ~PluginEntry() = default;
    PluginEntry(const PluginEntry&) = delete;
    PluginEntry& operator=(const PluginEntry&) = delete;

    bool authenticated() const;
    virtual void initialize();

    static std::unique_ptr<PluginEntry> create(QFileInfo&& fileInfo);
    static bool isPluginFile(const QString& fileName);
protected:
    explicit PluginEntry(QFileInfo&& fileInfo_) : fileInfo(std::move(fileInfo_)) {}
    void checkMetadata();
    void checkTargetVersion(std::string_view targetVersion);
private:
    QtTubePlugin::ProviderRegistry m_providerRegistry;
};
