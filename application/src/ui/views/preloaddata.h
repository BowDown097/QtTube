#pragma once
#include "innertube/objects/channel/metadatabadge.h"
#include "qttube-plugin/objects/badge.h"

namespace PreloadData
{
    struct WatchView
    {
        QString channelAvatarUrl;
        std::variant<QList<InnertubeObjects::MetadataBadge>, QList<QtTube::PluginBadge>> channelBadges;
        QString channelId;
        QString channelName;
        QString title;
    };
}
