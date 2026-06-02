#pragma once
#include "qjsutils.h"
#include <qttube-plugin/components/replytypes/channeldata.h>
#include <qttube-plugin/components/replytypes/initialaccountdata.h>
#include <qttube-plugin/components/replytypes/recommendedcontinuationdata.h>
#include <qttube-plugin/components/replytypes/resolveurldata.h>
#include <qttube-plugin/components/replytypes/videodata.h>
#include <qttube-plugin/objects/livechat/livechat.h>
#include <qttube-plugin/objects/livechat/livechatreplay.h>
#include <qttube-plugin/objects/notification.h>

#define UNWRAP_FIELD_PART(name) \
    QJSUtils::unwrapObjectProperty<decltype(T::name)>(ctx, val, #name)
#define UNWRAP_FIELD(name) .name = UNWRAP_FIELD_PART(name),
#define UNWRAP_NONAGGREGATE(name) result.name = UNWRAP_FIELD_PART(name);

#define WRAP_FIELD(name) \
    JS_SetPropertyStr(ctx, obj, #name, qjs::js_traits<std::decay_t<decltype(T::name)>>::wrap(ctx, val.name));

#define DEF_FOR(T) \
    template<> struct js_traits<T> \
    { \
        static T unwrap(JSContext* ctx, JSValueConst val); \
        static JSValue wrap(JSContext* ctx, const T& val); \
    };

namespace qjs
{
    DEF_FOR(QtTubePlugin::Badge)
    DEF_FOR(QtTubePlugin::Badge::ColorPalette)
    DEF_FOR(QtTubePlugin::Channel)
    DEF_FOR(QtTubePlugin::ChannelData)
    DEF_FOR(QtTubePlugin::ChannelHeader)
    DEF_FOR(QtTubePlugin::ChannelTabData)
    DEF_FOR(QtTubePlugin::Emoji)
    DEF_FOR(QtTubePlugin::GiftRedemptionMessage)
    DEF_FOR(QtTubePlugin::InitialAccountData)
    DEF_FOR(QtTubePlugin::InitialLiveChatData)
    DEF_FOR(QtTubePlugin::LiveChat)
    DEF_FOR(QtTubePlugin::LiveChatReplay)
    DEF_FOR(QtTubePlugin::LiveChatReplayItem)
    DEF_FOR(QtTubePlugin::LiveChatViewOption)
    DEF_FOR(QtTubePlugin::Notification)
    DEF_FOR(QtTubePlugin::NotificationBell)
    DEF_FOR(QtTubePlugin::NotificationState)
    DEF_FOR(QtTubePlugin::PaidMessage)
    DEF_FOR(QtTubePlugin::RecommendedContinuationData)
    DEF_FOR(QtTubePlugin::ResolveUrlData)
    DEF_FOR(QtTubePlugin::SpecialMessage)
    DEF_FOR(QtTubePlugin::SubscribeButton)
    DEF_FOR(QtTubePlugin::SubscribeButton::ColorPalette)
    DEF_FOR(QtTubePlugin::SubscribeButton::Localization)
    DEF_FOR(QtTubePlugin::TextMessage)
    DEF_FOR(QtTubePlugin::Video)
    DEF_FOR(QtTubePlugin::VideoData)
    DEF_FOR(QtTubePlugin::VideoData::Continuations)
    DEF_FOR(QtTubePlugin::VideoData::LikeData)

    // since this is a templated type, we have to define the full thing here
    template<typename... ItemTypes>
    struct js_traits<QtTubePlugin::Shelf<ItemTypes...>>
    {
        using T = QtTubePlugin::Shelf<ItemTypes...>;

        static T unwrap(JSContext* ctx, JSValueConst val)
        {
            T result;
            UNWRAP_NONAGGREGATE(contents)
            UNWRAP_NONAGGREGATE(iconUrl)
            UNWRAP_NONAGGREGATE(isDividerHidden)
            UNWRAP_NONAGGREGATE(subtitle)
            UNWRAP_NONAGGREGATE(title)
            return result;
        }

        static JSValue wrap(JSContext* ctx, const QtTubePlugin::Shelf<ItemTypes...>& val)
        {
            JSValue obj = JS_NewObject(ctx);
            WRAP_FIELD(contents)
            WRAP_FIELD(iconUrl)
            WRAP_FIELD(isDividerHidden)
            WRAP_FIELD(subtitle)
            WRAP_FIELD(title)
            return obj;
        }
    };
}

void registerEnumsFor(qjs::context& ctx);