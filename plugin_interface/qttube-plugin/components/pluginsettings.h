#pragma once
#include "configstore.h"

namespace QtTube
{
    struct PluginSettings : ConfigStore
    {
        virtual ~PluginSettings() = default;
        virtual QWidget* window() { return nullptr; }

        template<typename T> requires std::derived_from<T, PluginSettings>
        static std::unique_ptr<T> create(const QString& plugin)
        {
            return ConfigStore::create<T>(plugin, "settings");
        }
    };
}
