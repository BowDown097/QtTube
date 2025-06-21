#pragma once
#include "qttube-plugin/plugininterface.h"
#include "settings/youtubesettings.h"
#include "youtubeauth.h"

class YouTubePlugin : public QtTube::PluginInterface
{
public:
    QtTube::BrowseReply* getHistory(const QString& query, std::any continuationData) override;
    QtTube::BrowseReply* getHome(std::any continuationData) override;
    QtTube::NotificationsReply* getNotifications(std::any continuationData) override;
    QtTube::BrowseReply* getSearch(
        const QString& query,
        const QList<std::pair<QString, int>>& activeFilters,
        std::any continuationData) override;
    QtTube::BrowseReply* getSubFeed(std::any continuationData) override;
    QtTube::BrowseReply* getTrending(std::any continuationData) override;
    QtTube::VideoReply* getVideo(const QString& videoId, std::any continuationData) override;

    QtTube::LiveChatReply* getLiveChat(std::any data) override;
    QtTube::LiveChatReplayReply* getLiveChatReplay(std::any data, qint64 videoOffsetMs) override;
    QtTube::PluginReply<void>* sendLiveChatMessage(const QString& text) override;

    QtTube::PluginReply<void>* rate(const QString& videoId, bool like, bool removing, std::any data) override;

    QtTube::PluginReply<void>* setNotificationPreference(std::any data) override;
    QtTube::PluginReply<void>* subscribe(std::any data) override;
    QtTube::PluginReply<void>* unsubscribe(std::any data) override;

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
extern QtTube::PluginMetadata g_metadata;
extern YouTubeSettings* g_settings;
