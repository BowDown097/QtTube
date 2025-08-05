#pragma once
#include "channeldata.h"
#include "initialaccountdata.h"
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/livechat/livechat.h"
#include "qttube-plugin/objects/livechat/livechatreplay.h"
#include "recommendedcontinuationdata.h"
#include "resolveurldata.h"
#include "simpledatatypes.h"
#include "videodata.h"

W_REGISTER_ARGTYPE(QtTubePlugin::BrowseData)
W_REGISTER_ARGTYPE(QtTubePlugin::ChannelData)
W_REGISTER_ARGTYPE(QtTubePlugin::InitialAccountData)
W_REGISTER_ARGTYPE(QtTubePlugin::LiveChat)
W_REGISTER_ARGTYPE(QtTubePlugin::LiveChatReplay)
W_REGISTER_ARGTYPE(QtTubePlugin::NotificationsData)
W_REGISTER_ARGTYPE(QtTubePlugin::RecommendedContinuationData)
W_REGISTER_ARGTYPE(QtTubePlugin::ResolveUrlData)
W_REGISTER_ARGTYPE(QtTubePlugin::VideoData)

namespace QtTubePlugin
{
    using AccountReply = Reply<InitialAccountData>;
    using BrowseReply = Reply<BrowseData>;
    using ChannelReply = Reply<ChannelData>;
    using LiveChatReply = Reply<LiveChat>;
    using LiveChatReplayReply = Reply<LiveChatReplay>;
    using NotificationsReply = Reply<NotificationsData>;
    using RecommendedContinuationReply = Reply<RecommendedContinuationData>;
    using ResolveUrlReply = Reply<ResolveUrlData>;
    using VideoReply = Reply<VideoData>;
}
