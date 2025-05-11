#pragma once
#include "configstore.h"

namespace QtTube
{
    struct PluginAuth : ConfigStore
    {
        struct AuthUser
        {
            QString avatar;
            QString id;
            QString username;
        };

        virtual ~PluginAuth() = default;

        template<typename T> requires std::derived_from<T, PluginAuth>
        static std::unique_ptr<T> create(const QString& plugin)
        {
            return ConfigStore::create<T>(plugin, "auth");
        }
    };
}
