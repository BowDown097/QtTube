#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTube
{
    using BrowseDataItem = std::variant<PluginVideo, PluginShelf<PluginVideo>>;
    using BrowseData = QList<BrowseDataItem>;
}

W_REGISTER_ARGTYPE(QtTube::BrowseData)

namespace QtTube
{
    using BrowseReply = PluginReply<BrowseData>;
}
