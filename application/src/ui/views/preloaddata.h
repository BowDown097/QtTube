#pragma once
#include "qttube-plugin/objects/badge.h"
#include <QList>

namespace PreloadData
{
    struct WatchView
    {
        QString channelAvatarUrl;
        QList<QtTube::PluginBadge> channelBadges;
        QString channelId;
        QString channelName;
        QString title;
    };
}
