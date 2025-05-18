#pragma once
#include <QString>

namespace QtTube
{
    struct PluginNotification
    {
        enum class TargetType { Channel, Video };

        QString body;
        QString channelAvatarUrl;
        QString channelId;
        QString channelUrlPrefix;
        QString notificationId;
        QString sentTimeText;
        QString sourceIconUrl;
        QString targetId;
        TargetType targetType;
        QString targetUrlPrefix;
        QString thumbnailUrl;
    };
}
