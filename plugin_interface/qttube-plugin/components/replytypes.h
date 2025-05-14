#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTube
{
    using HomeDataItem = std::variant<PluginVideo, PluginShelf<PluginVideo>>;
    using HomeData = QList<HomeDataItem>;
}

W_REGISTER_ARGTYPE(QtTube::HomeData)

namespace QtTube
{
    using HomeReply = PluginReply<HomeData>;
}
