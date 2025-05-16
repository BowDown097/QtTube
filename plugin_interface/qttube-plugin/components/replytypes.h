#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTube
{
    using HomeDataItem = std::variant<PluginVideo, PluginShelf<PluginVideo>>;
    using HomeData = QList<HomeDataItem>;
    using TrendingDataItem = std::variant<PluginVideo, PluginShelf<PluginVideo>>;
    using TrendingData = QList<TrendingDataItem>;
}

W_REGISTER_ARGTYPE(QtTube::HomeData)
// W_REGISTER_ARGTYPE(QtTube::TrendingData) /* this is equal to HomeData, so we don't register it */

namespace QtTube
{
    using HomeReply = PluginReply<HomeData>;
    using TrendingReply = PluginReply<TrendingData>;
}
