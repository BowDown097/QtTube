#pragma once
#include "qttube-plugin/objects/channel.h"
#include "qttube-plugin/objects/notification.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTube
{
    using BrowseDataItem = std::variant<PluginChannel, PluginVideo, PluginShelf<PluginVideo>>;
    using BrowseData = QList<BrowseDataItem>;
    using NotificationsDataItem = PluginNotification;
    using NotificationsData = QList<NotificationsDataItem>;
}
