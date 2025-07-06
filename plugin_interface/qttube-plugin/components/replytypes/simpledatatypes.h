#pragma once
#include "qttube-plugin/objects/channel.h"
#include "qttube-plugin/objects/notification.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTubePlugin
{
    using BrowseDataItem = std::variant<Channel, Video, Shelf<Video>>;
    using BrowseData = QList<BrowseDataItem>;
    using NotificationsDataItem = Notification;
    using NotificationsData = QList<NotificationsDataItem>;
}
