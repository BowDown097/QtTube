#pragma once
#include <QString>

namespace QtTubePlugin
{
    struct InitialAccountData
    {
        QString avatarUrl;
        QString channelId;
        QString displayName;
        QString handle;
        int notificationCount{};
    };
}
