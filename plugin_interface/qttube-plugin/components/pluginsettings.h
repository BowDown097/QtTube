#pragma once
#include "configstore.h"
#include "pluginsettingswindow.h"

namespace QtTube
{
    struct PluginSettings : ConfigStore
    {
        virtual PluginSettingsWindow* window() { return nullptr; }

        template<typename T> requires std::derived_from<T, PluginSettings>
        static std::unique_ptr<T> create(const QString& plugin)
        {
            return ConfigStore::create<T>(plugin, "settings");
        }
    };
}
