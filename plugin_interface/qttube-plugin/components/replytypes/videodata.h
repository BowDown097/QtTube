#pragma once
#include "qttube-plugin/objects/channel.h"
#include "qttube-plugin/objects/emoji.h"
#include "qttube-plugin/objects/video.h"

namespace QtTubePlugin
{
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

        Channel channel;
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
        QList<Video> recommendedVideos;
        PluginMetadata* sourceMetadata{};
        QString titleText;
        QString videoId;
        QString videoUrlPrefix;
        QString viewCountText;
    };
}
