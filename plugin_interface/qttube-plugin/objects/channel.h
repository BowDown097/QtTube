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
        struct ColorPalette
        {
            QString subscribeBackground = "red";
            QString subscribeBorder = "transparent";
            QString subscribeForeground = "#fefefe";

            QString subscribeHoveredBackground = "#d90a17";
            QString subscribeHoveredBorder = "transparent";
            QString subscribeHoveredForeground = "#fefefe";

            QString subscribedBackground;
            QString subscribedBorder = "#555";
            QString subscribedForeground;

            QString unsubscribeBackground;
            QString unsubscribeBorder = "#555";
            QString unsubscribeForeground;
        };

        struct Localization
        {
            QString subscribeText;
            QString subscribedText;
            QString unsubscribeDialogText;
            QString unsubscribeText;
        };

        ColorPalette colorPalette;
        QString countText;
        bool enabled{};
        Localization localization;
        PluginNotificationBell notificationBell;
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
        PluginMetadata* sourceMetadata{};
        PluginSubscribeButton subscribeButton;
    };
}
