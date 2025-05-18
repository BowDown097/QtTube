#pragma once
#include <QString>

namespace QtTube
{
    struct PluginMetadata;

    struct PluginNotification
    {
        enum class TargetType { Channel, Video };

        QString body;
        QString channelAvatarUrl;
        QString channelId;
        QString channelUrlPrefix;
        QString notificationId;
        QString sentTimeText;
        PluginMetadata* sourceMetadata;
        QString targetId;
        TargetType targetType;
        QString targetUrlPrefix;
        QString thumbnailUrl;
    };
}
