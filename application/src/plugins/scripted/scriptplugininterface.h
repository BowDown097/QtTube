#pragma once
#include "scriptpluginauthstore.h"
#include <qttube-plugin/plugininterface.h>
#include <quickjs++/context.h>

class ScriptPluginInterface : public QtTubePlugin::PluginInterface
{
public:
    // custom methods
    explicit ScriptPluginInterface(
        const QString& pluginName,
        qjs::value&& moduleNamespace,
        std::unique_ptr<qjs::context>&& context);

    QtTubePlugin::AuthStoreBase* authStore() { return m_authStore.get(); }
    qjs::context* context() { return m_context.get(); }
    QtTubePluginPlayerFunc playerFunc() { return nullptr; }
    QtTubePlugin::SettingsStore* settings() { return m_settingsStore.get(); }

    // interface methods
    QtTubePlugin::RecommendedContinuationReply* continueRecommended(
        const QString& videoId, std::any continuationData) override;
    QtTubePlugin::AccountReply* getActiveAccount() override;
    QtTubePlugin::ChannelReply* getChannel(
        const QString& channelId, std::any tabData, std::any continuationData) override;
    QtTubePlugin::BrowseReply* getHistory(const QString& query, std::any continuationData) override;
    QtTubePlugin::BrowseReply* getHome(std::any continuationData) override;
    QtTubePlugin::LiveChatReply* getLiveChat(std::any data) override;
    QtTubePlugin::LiveChatReplayReply* getLiveChatReplay(std::any data, qint64 videoOffsetMs) override;
    QtTubePlugin::NotificationsReply* getNotifications(std::any continuationData) override;
    QtTubePlugin::BrowseReply* getSearch(
        const QString& query, const QList<std::pair<QString, int>>& activeFilters, std::any continuationData) override;
    QtTubePlugin::BrowseReply* getSubFeed(std::any continuationData) override;
    QtTubePlugin::BrowseReply* getTrending(std::any continutaionData) override;
    QtTubePlugin::VideoReply* getVideo(const QString& videoId) override;
    void init() override;
    QtTubePlugin::Reply<void>* rate(
        const QString& videoId, bool like, bool removing, std::any data) override;
    QtTubePlugin::ResolveUrlReply* resolveUrlOrID(const QString& in) override;
    const QList<std::pair<QString, QStringList>> searchFilters() const override;
    QtTubePlugin::Reply<void>* sendLiveChatMessage(const QString& text) override;
    QtTubePlugin::Reply<void>* setNotificationPreference(std::any data) override;
    QtTubePlugin::Reply<void>* subscribe(std::any data) override;
    QtTubePlugin::Reply<void>* unsubscribe(std::any data) override;
private:
    std::unique_ptr<ScriptPluginAuthStore> m_authStore;
    std::unique_ptr<qjs::context> m_context;
    qjs::value m_moduleNamespace;
    std::unique_ptr<QtTubePlugin::SettingsStore> m_settingsStore;
};
