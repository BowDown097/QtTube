#pragma once
#include "qttube-plugin/plugininterface.h"
#include "settings/youtubesettings.h"
#include "youtubeauth.h"

class YouTubePlugin : public QtTubePlugin::PluginInterface
{
public:
    QtTubePlugin::AccountReply* getActiveAccount() override;
    QtTubePlugin::ChannelReply* getChannel(const QString& channelId, std::any tabData, std::any continuationData) override;
    QtTubePlugin::BrowseReply* getHistory(const QString& query, std::any continuationData) override;
    QtTubePlugin::BrowseReply* getHome(std::any continuationData) override;
    QtTubePlugin::NotificationsReply* getNotifications(std::any continuationData) override;
    QtTubePlugin::BrowseReply* getSearch(
        const QString& query,
        const QList<std::pair<QString, int>>& activeFilters,
        std::any continuationData) override;
    QtTubePlugin::BrowseReply* getSubFeed(std::any continuationData) override;
    QtTubePlugin::BrowseReply* getTrending(std::any continuationData) override;
    QtTubePlugin::VideoReply* getVideo(const QString& videoId, std::any continuationData) override;

    QtTubePlugin::LiveChatReply* getLiveChat(std::any data) override;
    QtTubePlugin::LiveChatReplayReply* getLiveChatReplay(std::any data, qint64 videoOffsetMs) override;
    QtTubePlugin::Reply<void>* sendLiveChatMessage(const QString& text) override;

    QtTubePlugin::Reply<void>* rate(const QString& videoId, bool like, bool removing, std::any data) override;

    QtTubePlugin::Reply<void>* setNotificationPreference(std::any data) override;
    QtTubePlugin::Reply<void>* subscribe(std::any data) override;
    QtTubePlugin::Reply<void>* unsubscribe(std::any data) override;

    QtTubePlugin::ResolveUrlReply* resolveUrlOrID(const QString& in) override;

    void init() override;

    const QList<std::pair<QString, QStringList>> searchFilters() const override
    {
        return {
            { "Upload date", { "Last hour", "Today", "This week", "This month", "This year" } },
            { "Type", { "Video", "Channel", "Playlist", "Movie" } },
            { "Duration", { "Under 4 minutes", "Over 20 minutes", "4-20 minutes" } },
            { "Features", { "Live", "4K", "HD", "Subtitles/CC", "Creative Commons", "360°", "VR180", "3D", "HDR", "Location", "Purchased" } },
            { "Sort by", { "Relevance", "Rating", "Upload date", "View count" } }
        };
    }
private:
    QByteArray compileSearchParams(const QList<std::pair<QString, int>>& activeFilters);
};

extern YouTubeAuth* g_auth;
extern QtTubePlugin::PluginMetadata g_metadata;
extern YouTubeSettings* g_settings;
