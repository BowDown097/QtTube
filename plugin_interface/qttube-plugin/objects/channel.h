#pragma once
#include "qttube-plugin/objects/badge.h"
#include <any>
#include <QList>

namespace QtTube
{
    struct PluginMetadata;

    struct PluginSubscribeButton
    {
        QString countText;
        bool enabled{};
        bool subscribed{};
        std::any subscribeData;
        std::any unsubscribeData;
    };

    struct PluginChannel
    {
        QString channelAvatarUrl;
        QList<PluginBadge> channelBadges;
        QString channelId;
        QString channelName;
        QString channelUrlPrefix;
        QString description;
        QString metadataText;
        PluginMetadata* sourceMetadata;
        PluginSubscribeButton subscribeButton;
    };
}
