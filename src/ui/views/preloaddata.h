#pragma once
#include "innertube/objects/responsiveimage.h"

namespace InnertubeObjects { struct MetadataBadge; }

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
