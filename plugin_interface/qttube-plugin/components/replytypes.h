#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/channel.h"
#include "qttube-plugin/objects/emoji.h"
#include "qttube-plugin/objects/livechat/livechat.h"
#include "qttube-plugin/objects/livechat/livechatreplay.h"
#include "qttube-plugin/objects/notification.h"
#include "qttube-plugin/objects/shelf.h"
#include "qttube-plugin/objects/video.h"

namespace QtTube
{
    using BrowseDataItem = std::variant<PluginChannel, PluginVideo, PluginShelf<PluginVideo>>;
    using BrowseData = QList<BrowseDataItem>;
    using NotificationsDataItem = PluginNotification;
    using NotificationsData = QList<NotificationsDataItem>;

    struct InitialLiveChatData
    {
        std::any data;
        bool isReplay{};
        QList<Emoji> platformEmojis;
        int updateIntervalMs = 1000;
    };

    struct VideoData
    {
        struct Continuations
        {
            std::any comments;
            std::any recommended;
        };

        struct LikeData
        {
            std::any like;
            std::any removeLike;
            std::any dislike;
            std::any removeDislike;
        };

        enum class LikeStatus { Liked, Disliked, Neutral };

        PluginChannel channel;
        Continuations continuations;
        QString dateText;
        QString descriptionText;
        QString dislikeCountText;
        std::optional<InitialLiveChatData> initialLiveChatData;
        bool isLiveContent{};
        QString likeCountText;
        LikeData likeData;
        float likeDislikeRatio{};
        LikeStatus likeStatus = LikeStatus::Neutral;
        bool ratingsAvailable = true;
        QList<PluginVideo> recommendedVideos;
        PluginMetadata* sourceMetadata{};
        QString titleText;
        QString videoId;
        QString videoUrlPrefix;
        QString viewCountText;
    };
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
