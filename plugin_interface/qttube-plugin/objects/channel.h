#pragma once
#include "qttube-plugin/objects/badge.h"
#include <any>
#include <QList>

namespace QtTube
{
    struct PluginMetadata;

    struct PluginNotificationState
    {
        enum class Representation { All, None, Neutral };

        std::any data;
        QString name;
        Representation representation;
    };

    struct PluginNotificationBell
    {
        qsizetype activeStateIndex = -1;
        qsizetype defaultEnabledStateIndex = -1;
        QList<PluginNotificationState> states;
    };

    struct PluginSubscribeButton
    {
        // plain data
        QString countText;
        bool enabled{};
        PluginNotificationBell notificationBell;
        bool subscribed{};
        std::any subscribeData;
        std::any unsubscribeData;

        // localization data
        QString subscribeText;
        QString subscribedText;
        QString unsubscribeDialogText;
        QString unsubscribeText;
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
