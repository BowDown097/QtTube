#include "plugin_js_traits.h"
#include <quickjs++/context.h>

#define WRAP_FOR(Type, WRAP_BODY) \
    JSValue js_traits<Type>::wrap(JSContext* ctx, const Type& val) \
    { \
        using T = Type; \
        JSValue obj = JS_NewObject(ctx); \
        WRAP_BODY \
        return obj; \
    }

#define TRAIT_FOR(Type, UNWRAP_BODY, WRAP_BODY) \
    Type js_traits<Type>::unwrap(JSContext* ctx, JSValueConst val) \
    { \
        using T = Type; \
        return { UNWRAP_BODY }; \
    } \
    WRAP_FOR(Type, WRAP_BODY)

#define TRAIT_FOR_NONAGGREGATE(Type, UNWRAP_BODY, WRAP_BODY) \
    Type js_traits<Type>::unwrap(JSContext* ctx, JSValueConst val) \
    { \
        using T = Type; \
        T result; \
        UNWRAP_BODY \
        return result; \
    } \
    WRAP_FOR(Type, WRAP_BODY)

namespace qjs
{
    TRAIT_FOR(QtTubePlugin::Badge,
        UNWRAP_FIELD(colorPalette)
        UNWRAP_FIELD(label)
        UNWRAP_FIELD(tooltip),

        WRAP_FIELD(colorPalette)
        WRAP_FIELD(label)
        WRAP_FIELD(tooltip)
    )
    TRAIT_FOR(QtTubePlugin::Badge::ColorPalette,
        UNWRAP_FIELD(background)
        UNWRAP_FIELD(foreground)
        UNWRAP_FIELD(hoveredBackground)
        UNWRAP_FIELD(hoveredForeground),

        WRAP_FIELD(background)
        WRAP_FIELD(foreground)
        WRAP_FIELD(hoveredBackground)
        WRAP_FIELD(hoveredForeground)
    )
    TRAIT_FOR(QtTubePlugin::Channel,
        UNWRAP_FIELD(channelAvatarUrl)
        UNWRAP_FIELD(channelBadges)
        UNWRAP_FIELD(channelId)
        UNWRAP_FIELD(channelName)
        UNWRAP_FIELD(description)
        UNWRAP_FIELD(metadataText)
        UNWRAP_FIELD(subscribeButton),

        WRAP_FIELD(channelAvatarUrl)
        WRAP_FIELD(channelBadges)
        WRAP_FIELD(channelId)
        WRAP_FIELD(channelName)
        WRAP_FIELD(description)
        WRAP_FIELD(metadataText)
        WRAP_FIELD(subscribeButton)
    )
    TRAIT_FOR(QtTubePlugin::ChannelData,
        UNWRAP_FIELD(header)
        UNWRAP_FIELD(tabs),

        WRAP_FIELD(header)
        WRAP_FIELD(tabs)
    )
    TRAIT_FOR(QtTubePlugin::ChannelHeader,
        UNWRAP_FIELD(avatarUrl)
        UNWRAP_FIELD(bannerUrl)
        UNWRAP_FIELD(channelSubtext)
        UNWRAP_FIELD(channelText)
        UNWRAP_FIELD(subscribeButton),

        WRAP_FIELD(avatarUrl)
        WRAP_FIELD(bannerUrl)
        WRAP_FIELD(channelSubtext)
        WRAP_FIELD(channelText)
        WRAP_FIELD(subscribeButton)
    )
    TRAIT_FOR(QtTubePlugin::ChannelTabData,
        UNWRAP_FIELD(items)
        UNWRAP_FIELD(requestData)
        UNWRAP_FIELD(title),

        WRAP_FIELD(items)
        WRAP_FIELD(requestData)
        WRAP_FIELD(title)
    )
    TRAIT_FOR(QtTubePlugin::Emoji,
        UNWRAP_FIELD(emoticons)
        UNWRAP_FIELD(hidden)
        UNWRAP_FIELD(representation)
        UNWRAP_FIELD(shortcodes)
        UNWRAP_FIELD(url),

        WRAP_FIELD(emoticons)
        WRAP_FIELD(hidden)
        WRAP_FIELD(representation)
        WRAP_FIELD(shortcodes)
        WRAP_FIELD(url)
    )
    TRAIT_FOR(QtTubePlugin::GiftRedemptionMessage,
        UNWRAP_FIELD(authorName)
        UNWRAP_FIELD(content),

        WRAP_FIELD(authorName)
        WRAP_FIELD(content)
    )
    TRAIT_FOR(QtTubePlugin::InitialAccountData,
        UNWRAP_FIELD(avatarUrl)
        UNWRAP_FIELD(channelId)
        UNWRAP_FIELD(displayName)
        UNWRAP_FIELD(handle)
        UNWRAP_FIELD(notificationCount),

        WRAP_FIELD(avatarUrl)
        WRAP_FIELD(channelId)
        WRAP_FIELD(displayName)
        WRAP_FIELD(handle)
        WRAP_FIELD(notificationCount)
    )
    TRAIT_FOR(QtTubePlugin::InitialLiveChatData,
        UNWRAP_FIELD(data)
        UNWRAP_FIELD(isReplay)
        UNWRAP_FIELD(platformEmojis)
        UNWRAP_FIELD(updateIntervalMs),

        WRAP_FIELD(data)
        WRAP_FIELD(isReplay)
        WRAP_FIELD(platformEmojis)
        WRAP_FIELD(updateIntervalMs)
    )
    TRAIT_FOR_NONAGGREGATE(QtTubePlugin::LiveChat,
        UNWRAP_NONAGGREGATE(items)
        UNWRAP_NONAGGREGATE(nextData)
        UNWRAP_NONAGGREGATE(restrictedMessage)
        UNWRAP_NONAGGREGATE(viewOptions),

        WRAP_FIELD(items)
        WRAP_FIELD(nextData)
        WRAP_FIELD(restrictedMessage)
        WRAP_FIELD(viewOptions)
    )
    TRAIT_FOR_NONAGGREGATE(QtTubePlugin::LiveChatReplay,
        UNWRAP_NONAGGREGATE(items)
        UNWRAP_NONAGGREGATE(nextData)
        UNWRAP_NONAGGREGATE(seekData)
        UNWRAP_NONAGGREGATE(viewOptions),

        WRAP_FIELD(items)
        WRAP_FIELD(nextData)
        WRAP_FIELD(seekData)
        WRAP_FIELD(viewOptions)
    )
    TRAIT_FOR(QtTubePlugin::LiveChatReplayItem,
        UNWRAP_FIELD(item)
        UNWRAP_FIELD(videoOffsetMs),

        WRAP_FIELD(item)
        WRAP_FIELD(videoOffsetMs)
    )
    TRAIT_FOR(QtTubePlugin::LiveChatViewOption,
        UNWRAP_FIELD(name)
        UNWRAP_FIELD(data),

        WRAP_FIELD(name)
        WRAP_FIELD(data)
    )
    TRAIT_FOR(QtTubePlugin::Notification,
        UNWRAP_FIELD(body)
        UNWRAP_FIELD(channelAvatarUrl)
        UNWRAP_FIELD(channelId)
        UNWRAP_FIELD(notificationId)
        UNWRAP_FIELD(sentTimeText)
        UNWRAP_FIELD(targetId)
        UNWRAP_FIELD(targetType)
        UNWRAP_FIELD(thumbnailUrl),

        WRAP_FIELD(body)
        WRAP_FIELD(channelAvatarUrl)
        WRAP_FIELD(channelId)
        WRAP_FIELD(notificationId)
        WRAP_FIELD(sentTimeText)
        WRAP_FIELD(targetId)
        WRAP_FIELD(targetType)
        WRAP_FIELD(thumbnailUrl)
    )
    TRAIT_FOR(QtTubePlugin::NotificationBell,
        UNWRAP_FIELD(activeStateIndex)
        UNWRAP_FIELD(defaultEnabledStateIndex)
        UNWRAP_FIELD(states),

        WRAP_FIELD(activeStateIndex)
        WRAP_FIELD(defaultEnabledStateIndex)
        WRAP_FIELD(states)
    )
    TRAIT_FOR(QtTubePlugin::NotificationState,
        UNWRAP_FIELD(data)
        UNWRAP_FIELD(name)
        UNWRAP_FIELD(representation),

        WRAP_FIELD(data)
        WRAP_FIELD(name)
        WRAP_FIELD(representation)
    )
    TRAIT_FOR(QtTubePlugin::PaidMessage,
        UNWRAP_FIELD(authorAvatarUrl)
        UNWRAP_FIELD(authorName)
        UNWRAP_FIELD(content)
        UNWRAP_FIELD(contentBackgroundColor)
        UNWRAP_FIELD(contentTextColor)
        UNWRAP_FIELD(headerBackgroundColor)
        UNWRAP_FIELD(headerTextColor)
        UNWRAP_FIELD(paidAmountText),

        WRAP_FIELD(authorAvatarUrl)
        WRAP_FIELD(authorName)
        WRAP_FIELD(content)
        WRAP_FIELD(contentBackgroundColor)
        WRAP_FIELD(contentTextColor)
        WRAP_FIELD(headerBackgroundColor)
        WRAP_FIELD(headerTextColor)
        WRAP_FIELD(paidAmountText)
    )
    TRAIT_FOR(QtTubePlugin::RecommendedContinuationData,
        UNWRAP_FIELD(nextContinuation)
        UNWRAP_FIELD(videos),

        WRAP_FIELD(nextContinuation)
        WRAP_FIELD(videos)
    )
    TRAIT_FOR(QtTubePlugin::ResolveUrlData,
        UNWRAP_FIELD(continuePlayback)
        UNWRAP_FIELD(data)
        UNWRAP_FIELD(input)
        UNWRAP_FIELD(target)
        UNWRAP_FIELD(videoProgress),

        WRAP_FIELD(continuePlayback)
        WRAP_FIELD(data)
        WRAP_FIELD(input)
        WRAP_FIELD(target)
        WRAP_FIELD(videoProgress)
    )
    TRAIT_FOR(QtTubePlugin::SpecialMessage,
        UNWRAP_FIELD(backgroundColor)
        UNWRAP_FIELD(content)
        UNWRAP_FIELD(contentStyle)
        UNWRAP_FIELD(header)
        UNWRAP_FIELD(headerStyle),

        WRAP_FIELD(backgroundColor)
        WRAP_FIELD(content)
        WRAP_FIELD(contentStyle)
        WRAP_FIELD(header)
        WRAP_FIELD(headerStyle)
    )
    TRAIT_FOR(QtTubePlugin::SubscribeButton,
        UNWRAP_FIELD(colorPalette)
        UNWRAP_FIELD(countText)
        UNWRAP_FIELD(enabled)
        UNWRAP_FIELD(localization)
        UNWRAP_FIELD(notificationBell)
        UNWRAP_FIELD(subscribed)
        UNWRAP_FIELD(subscribeData)
        UNWRAP_FIELD(unsubscribeData),

        WRAP_FIELD(colorPalette)
        WRAP_FIELD(countText)
        WRAP_FIELD(enabled)
        WRAP_FIELD(localization)
        WRAP_FIELD(notificationBell)
        WRAP_FIELD(subscribed)
        WRAP_FIELD(subscribeData)
        WRAP_FIELD(unsubscribeData)
    )
    TRAIT_FOR(QtTubePlugin::SubscribeButton::ColorPalette,
        UNWRAP_FIELD(subscribeBackground)
        UNWRAP_FIELD(subscribeBorder)
        UNWRAP_FIELD(subscribeForeground)
        UNWRAP_FIELD(subscribeDisabledBackground)
        UNWRAP_FIELD(subscribeDisabledBorder)
        UNWRAP_FIELD(subscribeDisabledForeground)
        UNWRAP_FIELD(subscribeHoveredBackground)
        UNWRAP_FIELD(subscribeHoveredBorder)
        UNWRAP_FIELD(subscribeHoveredForeground)
        UNWRAP_FIELD(subscribedBackground)
        UNWRAP_FIELD(subscribedBorder)
        UNWRAP_FIELD(subscribedForeground)
        UNWRAP_FIELD(unsubscribeBackground)
        UNWRAP_FIELD(unsubscribeBorder)
        UNWRAP_FIELD(unsubscribeForeground),

        WRAP_FIELD(subscribeBackground)
        WRAP_FIELD(subscribeBorder)
        WRAP_FIELD(subscribeForeground)
        WRAP_FIELD(subscribeDisabledBackground)
        WRAP_FIELD(subscribeDisabledBorder)
        WRAP_FIELD(subscribeDisabledForeground)
        WRAP_FIELD(subscribeHoveredBackground)
        WRAP_FIELD(subscribeHoveredBorder)
        WRAP_FIELD(subscribeHoveredForeground)
        WRAP_FIELD(subscribedBackground)
        WRAP_FIELD(subscribedBorder)
        WRAP_FIELD(subscribedForeground)
        WRAP_FIELD(unsubscribeBackground)
        WRAP_FIELD(unsubscribeBorder)
        WRAP_FIELD(unsubscribeForeground)
    )
    TRAIT_FOR(QtTubePlugin::SubscribeButton::Localization,
        UNWRAP_FIELD(subscribeText)
        UNWRAP_FIELD(subscribedText)
        UNWRAP_FIELD(unsubscribeDialogText)
        UNWRAP_FIELD(unsubscribeText),

        WRAP_FIELD(subscribeText)
        WRAP_FIELD(subscribedText)
        WRAP_FIELD(unsubscribeDialogText)
        WRAP_FIELD(unsubscribeText)
    )
    TRAIT_FOR(QtTubePlugin::TextMessage,
        UNWRAP_FIELD(authorAvatarUrl)
        UNWRAP_FIELD(authorName)
        UNWRAP_FIELD(authorNameColor)
        UNWRAP_FIELD(content)
        UNWRAP_FIELD(timestampText),

        WRAP_FIELD(authorAvatarUrl)
        WRAP_FIELD(authorName)
        WRAP_FIELD(authorNameColor)
        WRAP_FIELD(content)
        WRAP_FIELD(timestampText)
    )
    TRAIT_FOR(QtTubePlugin::Video,
        UNWRAP_FIELD(badges)
        UNWRAP_FIELD(isVerticalVideo)
        UNWRAP_FIELD(lengthText)
        UNWRAP_FIELD(metadataText)
        UNWRAP_FIELD(progressSecs)
        UNWRAP_FIELD(thumbnailUrl)
        UNWRAP_FIELD(title)
        UNWRAP_FIELD(uploaderAvatarUrl)
        UNWRAP_FIELD(uploaderBadges)
        UNWRAP_FIELD(uploaderId)
        UNWRAP_FIELD(uploaderText)
        UNWRAP_FIELD(videoId),

        WRAP_FIELD(badges)
        WRAP_FIELD(isVerticalVideo)
        WRAP_FIELD(lengthText)
        WRAP_FIELD(metadataText)
        WRAP_FIELD(progressSecs)
        WRAP_FIELD(thumbnailUrl)
        WRAP_FIELD(title)
        WRAP_FIELD(uploaderAvatarUrl)
        WRAP_FIELD(uploaderBadges)
        WRAP_FIELD(uploaderId)
        WRAP_FIELD(uploaderText)
        WRAP_FIELD(videoId)
    )
    TRAIT_FOR(QtTubePlugin::VideoData,
        UNWRAP_FIELD(channel)
        UNWRAP_FIELD(continuations)
        UNWRAP_FIELD(dateText)
        UNWRAP_FIELD(descriptionText)
        UNWRAP_FIELD(dislikeCountText)
        UNWRAP_FIELD(initialLiveChatData)
        UNWRAP_FIELD(isLiveContent)
        UNWRAP_FIELD(likeCountText)
        UNWRAP_FIELD(likeData)
        UNWRAP_FIELD(likeDislikeRatio)
        UNWRAP_FIELD(likeStatus)
        UNWRAP_FIELD(ratingsAvailable)
        UNWRAP_FIELD(recommendedVideos)
        UNWRAP_FIELD(titleText)
        UNWRAP_FIELD(videoId)
        UNWRAP_FIELD(viewCountText),

        WRAP_FIELD(channel)
        WRAP_FIELD(continuations)
        WRAP_FIELD(dateText)
        WRAP_FIELD(descriptionText)
        WRAP_FIELD(dislikeCountText)
        WRAP_FIELD(initialLiveChatData)
        WRAP_FIELD(isLiveContent)
        WRAP_FIELD(likeCountText)
        WRAP_FIELD(likeData)
        WRAP_FIELD(likeDislikeRatio)
        WRAP_FIELD(likeStatus)
        WRAP_FIELD(ratingsAvailable)
        WRAP_FIELD(recommendedVideos)
        WRAP_FIELD(titleText)
        WRAP_FIELD(videoId)
        WRAP_FIELD(viewCountText)
    )
    TRAIT_FOR(QtTubePlugin::VideoData::Continuations,
        UNWRAP_FIELD(comments)
        UNWRAP_FIELD(recommended),

        WRAP_FIELD(comments)
        WRAP_FIELD(recommended)
    )
    TRAIT_FOR(QtTubePlugin::VideoData::LikeData,
        UNWRAP_FIELD(like)
        UNWRAP_FIELD(removeLike)
        UNWRAP_FIELD(dislike)
        UNWRAP_FIELD(removeDislike),

        WRAP_FIELD(like)
        WRAP_FIELD(removeLike)
        WRAP_FIELD(dislike)
        WRAP_FIELD(removeDislike)
    )
}

void registerEnumsFor(qjs::context& ctx)
{
    using LikeStatus = QtTubePlugin::VideoData::LikeStatus;
    using Representation = QtTubePlugin::NotificationState::Representation;
    using ResolveUrlTarget = QtTubePlugin::ResolveUrlTarget;
    using TargetType = QtTubePlugin::Notification::TargetType;

    qjs::enum_registrar<Representation>("NotificationStateRepresentation", ctx)
        .value<Representation::All>()
        .value<Representation::None>()
        .value<Representation::Neutral>();

    qjs::enum_registrar<TargetType>("NotificationTargetType", ctx)
        .value<TargetType::Channel>()
        .value<TargetType::Video>();

    qjs::enum_registrar<ResolveUrlTarget>("ResolveUrlTarget", ctx)
        .value<ResolveUrlTarget::NotResolved>()
        .value<ResolveUrlTarget::PlainUrl>()
        .value<ResolveUrlTarget::Video>()
        .value<ResolveUrlTarget::Channel>()
        .value<ResolveUrlTarget::Search>();

    qjs::enum_registrar<LikeStatus>("VideoLikeStatus", ctx)
        .value<LikeStatus::Liked>()
        .value<LikeStatus::Disliked>()
        .value<LikeStatus::Neutral>();
}