#pragma once
#include "qttube-plugin/plugininterface.h"

class YouTubeSettings : public QtTube::PluginSettings
{
    void init() override;
    void save() override;
};
