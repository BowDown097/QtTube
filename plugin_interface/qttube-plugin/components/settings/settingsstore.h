#pragma once
#include "qttube-plugin/components/configstore.h"
#include "settingswindow.h"

namespace QtTubePlugin
{
    struct SettingsStore : ConfigStore
    {
        virtual SettingsWindow* window() { return nullptr; }

        template<typename T> requires std::derived_from<T, SettingsStore>
        static std::unique_ptr<T> create(const QString& plugin)
        {
            return ConfigStore::create<T>(plugin, "settings");
        }
    };
}
