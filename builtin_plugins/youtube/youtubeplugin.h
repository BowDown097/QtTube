#pragma once
#include "qttube-plugin/plugininterface.h"
#include "settings/youtubesettings.h"
#include "youtubeauth.h"

class YouTubePlugin : public QtTube::PluginInterface
{
    QtTube::BrowseReply* getHome(std::any data = {}) override;
    QtTube::BrowseReply* getSubFeed(std::any data = {}) override;
    QtTube::BrowseReply* getTrending(std::any data = {}) override;
    void init() override;
};

extern YouTubeAuth* g_auth;
extern QtTube::PluginMetadata g_metadata;
extern YouTubeSettings* g_settings;
