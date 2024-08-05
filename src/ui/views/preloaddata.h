#pragma once
#include "innertube/objects/channel/metadatabadge.h"
#include "innertube/objects/responsiveimage.h"

namespace PreloadData
{
    struct WatchView
    {
        std::optional<InnertubeObjects::ResponsiveImage> channelAvatar;
        QList<InnertubeObjects::MetadataBadge> channelBadges;
        std::optional<QString> channelId;
        std::optional<QString> channelName;
        std::optional<QString> title;
    };
}
