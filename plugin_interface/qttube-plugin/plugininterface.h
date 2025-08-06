#pragma once
#include "components/auth/authstore.h"
#include "components/player/player.h"
#include "components/replytypes/replytypes.h"
#include "components/settings/settingsstore.h"

#ifdef Q_OS_WIN
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

namespace QtTubePlugin
{
    struct PluginInterface
    {
        virtual ~PluginInterface() = default;

        virtual AccountReply* getActiveAccount() { return AccountReply::create(); }

        virtual ChannelReply* getChannel(
            const QString& channelId,
            std::any tabData,
            std::any continuationData) { return ChannelReply::create(); }

        virtual BrowseReply* getHistory(const QString& query, std::any continuationData) { return BrowseReply::create(); }
        virtual BrowseReply* getHome(std::any continuationData) { return BrowseReply::create(); }
        virtual NotificationsReply* getNotifications(std::any continuationData) { return NotificationsReply::create(); }
        virtual BrowseReply* getSearch(
            const QString& query,
            const QList<std::pair<QString, int>>& activeFilters, // mapped as category -> index
            std::any continuationData) { return BrowseReply::create(); }
        virtual BrowseReply* getSubFeed(std::any continuationData) { return BrowseReply::create(); }
        virtual BrowseReply* getTrending(std::any continuationData) { return BrowseReply::create(); }

        virtual RecommendedContinuationReply* continueRecommended(const QString& videoId, std::any continuationData)
        { return RecommendedContinuationReply::create(); }
        virtual VideoReply* getVideo(const QString& videoId) { return VideoReply::create(); }

        virtual LiveChatReply* getLiveChat(std::any data) { return LiveChatReply::create(); }
        virtual LiveChatReplayReply* getLiveChatReplay(std::any data, qint64 videoOffsetMs) { return LiveChatReplayReply::create(); }
        virtual Reply<void>* sendLiveChatMessage(const QString& text) { return Reply<void>::create(); }

        virtual Reply<void>* rate(
            const QString& videoId,
            bool like,
            bool removing,
            std::any data) { return Reply<void>::create(); }

        virtual Reply<void>* setNotificationPreference(std::any data) { return Reply<void>::create(); }
        virtual Reply<void>* subscribe(std::any data) { return Reply<void>::create(); }
        virtual Reply<void>* unsubscribe(std::any data) { return Reply<void>::create(); }

        virtual ResolveUrlReply* resolveUrlOrID(const QString& in) { return ResolveUrlReply::create(); }

        virtual void init() = 0;

        // mapped as category -> filters
        virtual const QList<std::pair<QString, QStringList>> searchFilters() const { return {}; }
    };

    struct PluginMetadata
    {
        const char* name = "";
        const char* version = "";
        const char* description = "";
        const char* image = "";
        const char* author = "";
        const char* url = "";
    };
}

using QtTubePluginAuthFunc = QtTubePlugin::AuthStoreBase*(*)();
using QtTubePluginMetadataFunc = QtTubePlugin::PluginMetadata*(*)();
using QtTubePluginNewInstanceFunc = QtTubePlugin::PluginInterface*(*)();
using QtTubePluginPlayerFunc = QtTubePlugin::Player*(*)(QWidget*);
using QtTubePluginSettingsFunc = QtTubePlugin::SettingsStore*(*)();
using QtTubePluginTargetVersionFunc = const char*(*)();

#define EXPAND(x) x
#define GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME
#define DECLARE_QTTUBE_PLUGIN(...) \
EXPAND(GET_MACRO(__VA_ARGS__, DECLARE_QTTUBE_PLUGIN5, \
                 DECLARE_QTTUBE_PLUGIN4, DECLARE_QTTUBE_PLUGIN3, \
                 DECLARE_QTTUBE_PLUGIN2, DECLARE_QTTUBE_PLUGIN1)(__VA_ARGS__))

#define DECLARE_QTTUBE_PLUGIN2(PluginClass, MetadataInstance) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata* metadata() { return &MetadataInstance; } \
    }

#define DECLARE_QTTUBE_PLUGIN3(PluginClass, MetadataInstance, PlayerClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata* metadata() { return &MetadataInstance; } \
        DLLEXPORT QtTubePlugin::Player* player(QWidget* parent) { return new PlayerClass(parent); } \
    }

#define DECLARE_QTTUBE_PLUGIN4(PluginClass, MetadataInstance, PlayerClass, SettingsClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata* metadata() { return &MetadataInstance; } \
        DLLEXPORT QtTubePlugin::Player* player(QWidget* parent) { return new PlayerClass(parent); } \
        DLLEXPORT QtTubePlugin::Settings* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTubePlugin::SettingsStore::create<SettingsClass>(metadata()->name); \
            return s.get(); \
        } \
    }

#define DECLARE_QTTUBE_PLUGIN5(PluginClass, MetadataInstance, PlayerClass, SettingsClass, AuthClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata* metadata() { return &MetadataInstance; } \
        DLLEXPORT QtTubePlugin::Player* player(QWidget* parent) { return new PlayerClass(parent); } \
        DLLEXPORT QtTubePlugin::SettingsStore* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTubePlugin::SettingsStore::create<SettingsClass>(metadata()->name); \
            return s.get(); \
        } \
        DLLEXPORT QtTubePlugin::AuthStoreBase* auth() \
        { \
            static std::unique_ptr<AuthClass> a = QtTubePlugin::AuthStoreBase::create<AuthClass>(metadata()->name); \
            return a.get(); \
        } \
    }
