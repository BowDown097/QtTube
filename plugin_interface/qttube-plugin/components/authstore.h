#pragma once
#include "configstore.h"

namespace QtTubePlugin
{
    struct AuthStore : ConfigStore
    {
        struct AuthUser
        {
            bool active{};
            QString avatar;
            QString id;
            QString username;

            AuthUser() = default;
            virtual ~AuthUser() = default;

            AuthUser(bool active, const QString& avatar, const QString& id, const QString& username)
                : active(active), avatar(avatar), id(id), username(username) {}
        };

        virtual const AuthUser* activeLogin() const = 0;

        template<typename T> requires std::derived_from<T, AuthStore>
        static std::unique_ptr<T> create(const QString& plugin)
        {
            return ConfigStore::create<T>(plugin, "auth");
        }
    };
}
