#pragma once
#include "qttube-plugin/plugininterface.h"
#include "settings/youtubesettings.h"
#include "youtubeauth.h"

class YouTubePlugin : public QtTube::PluginInterface
{
    QtTube::HomeReply* getHome() override;
    void init() override;
};

extern YouTubeAuth* g_auth;
extern QtTube::PluginMetadata g_metadata;
extern YouTubeSettings* g_settings;
