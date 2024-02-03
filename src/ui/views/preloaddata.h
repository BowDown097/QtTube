#ifndef PRELOADDATA_H
#define PRELOADDATA_H
#include "innertube/objects/responsiveimage.h"

namespace InnertubeObjects { class MetadataBadge; }

namespace PreloadData
{
    struct WatchView
    {
        std::optional<InnertubeObjects::ResponsiveImage> channelAvatar;
        QList<InnertubeObjects::MetadataBadge> channelBadges;
        std::optional<QString> channelName;
        std::optional<QString> title;
    };
}

#endif // PRELOADDATA_H
