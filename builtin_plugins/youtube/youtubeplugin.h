#pragma once
#include "qttube-plugin/plugininterface.h"
#include "settings/youtubesettings.h"
#include "youtubeauth.h"

class YouTubePlugin : public QtTube::PluginInterface
{
    QtTube::BrowseReply* getHistory(const QString& query = {}, std::any continuationData = {}) override;
    QtTube::BrowseReply* getHome(std::any continuationData = {}) override;
    QtTube::BrowseReply* getSubFeed(std::any continuationData = {}) override;
    QtTube::BrowseReply* getTrending(std::any continuationData = {}) override;
    void init() override;
};

extern YouTubeAuth* g_auth;
extern QtTube::PluginMetadata g_metadata;
extern YouTubeSettings* g_settings;
