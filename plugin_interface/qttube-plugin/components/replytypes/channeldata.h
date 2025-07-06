#pragma once
#include "qttube-plugin/objects/channel.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTubePlugin
{
    struct ChannelHeader
    {
        QString avatarUrl;
        QString bannerUrl;
        QString channelSubtext;
        QString channelText;
        SubscribeButton subscribeButton;
    };

    using ChannelTabDataItem = std::variant<
        std::monostate,
        Channel,
        Video,
        Shelf<Channel>,
        Shelf<Video>>;

    struct ChannelTabData
    {
        QList<ChannelTabDataItem> items;
        std::any requestData;
        QString title;
    };

    struct ChannelData
    {
        std::optional<ChannelHeader> header; // needs to be available initially, but may not be afterward
        QList<ChannelTabData> tabs;
    };
}
