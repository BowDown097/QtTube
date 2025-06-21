#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/livechat/livechat.h"
#include "qttube-plugin/objects/livechat/livechatreplay.h"
#include "qttube-plugin/objects/notification.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/videodata.h"

namespace QtTube
{
    using BrowseDataItem = std::variant<PluginChannel, PluginVideo, PluginShelf<PluginVideo>>;
    using BrowseData = QList<BrowseDataItem>;
    using NotificationsDataItem = PluginNotification;
    using NotificationsData = QList<NotificationsDataItem>;
}

W_REGISTER_ARGTYPE(QtTube::BrowseData)
W_REGISTER_ARGTYPE(QtTube::LiveChat)
W_REGISTER_ARGTYPE(QtTube::LiveChatReplay)
W_REGISTER_ARGTYPE(QtTube::NotificationsData)
W_REGISTER_ARGTYPE(QtTube::VideoData)

namespace QtTube
{
    using BrowseReply = PluginReply<BrowseData>;
    using LiveChatReply = PluginReply<LiveChat>;
    using LiveChatReplayReply = PluginReply<LiveChatReplay>;
    using NotificationsReply = PluginReply<NotificationsData>;
    using VideoReply = PluginReply<VideoData>;
}
