#pragma once
#include "qttube-plugin/plugininterface.h"

class YouTubePlugin : public QtTube::PluginInterface
{
    QtTube::HomeReply* getHome() override;
    void init() override;
};
