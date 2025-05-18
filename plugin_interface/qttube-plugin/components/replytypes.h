#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/notification.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTube
{
    using BrowseDataItem = std::variant<PluginVideo, PluginShelf<PluginVideo>>;
    using BrowseData = QList<BrowseDataItem>;
    using NotificationsDataItem = PluginNotification;
    using NotificationsData = QList<NotificationsDataItem>;
}

W_REGISTER_ARGTYPE(QtTube::BrowseData)
W_REGISTER_ARGTYPE(QtTube::NotificationsData)

namespace QtTube
{
    using BrowseReply = PluginReply<BrowseData>;
    using NotificationsReply = PluginReply<NotificationsData>;
}
