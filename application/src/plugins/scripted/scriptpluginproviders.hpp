#pragma once
#include "scriptpluginauthstore.hpp"
#include "utils/quickjs/plugin_js_traits.hpp"
#include <qttube-plugin/providers/providertypes.h>
#include <qttube-plugin/utils/replyutils.h>

JSValue anyValue(const std::any& value)
{
    try { return std::any_cast<const qjs::value&>(value).v; }
    catch (const std::bad_any_cast&) { return JS_NULL; }
}

template<typename T>
QtTubePlugin::Reply<T>* makeReply(const qjs::value& func, auto&&... args)
{
    using ReplyType = QtTubePlugin::Reply<T>;
    ReplyType* reply = ReplyType::create();

    try
    {
        if constexpr (std::is_void_v<T>)
        {
            func.invoke_then([=] {
                QtTubePlugin::invokeQueued(reply, &ReplyType::finished);
            }, std::forward<decltype(args)>(args)...);
        }
        else
        {
            func.invoke_then([=](const qjs::value& data) {
                if (qjs::value contData = data["continuationData"];
                    !contData.is_null() && !contData.is_undefined())
                {
                    reply->continuationData = contData;
                    QtTubePlugin::invokeQueued(reply, &ReplyType::finished, data["data"].as<T>());
                }
                else
                {
                    QtTubePlugin::invokeQueued(reply, &ReplyType::finished, data.as<T>());
                }
            }, std::forward<decltype(args)>(args)...);
        }
    }
    catch (const qjs::exception& ex)
    {
        QtTubePlugin::invokeQueued(reply, &ReplyType::exception,
            QtTubePlugin::Exception(QJSUtils::generateErrorString(ex)));
    }

    return reply;
}

struct ScriptAuthenticationProvider : QtTubePlugin::AuthenticationProvider
{
    std::unique_ptr<ScriptPluginAuthStore> authStore;
    qjs::value getActiveAccount_v;

    ScriptAuthenticationProvider(std::unique_ptr<ScriptPluginAuthStore> authStore, qjs::value&& getActiveAccount_v)
        : authStore(std::move(authStore)), getActiveAccount_v(std::move(getActiveAccount_v)) {}

    QtTubePlugin::AccountReply* getActiveAccount() override
    {
        if (QPointer<ScriptPluginAuthRoutine> routine = authStore->activeRoutine())
            return makeReply<QtTubePlugin::InitialAccountData>(getActiveAccount_v, routine->searchCookies(), routine->searchHeaders());
        else if (ScriptPluginAuthUser* login = authStore->activeLogin())
            return makeReply<QtTubePlugin::InitialAccountData>(getActiveAccount_v, login->cookies, login->headers);
        else
            return nullptr;
    }

    QtTubePlugin::AuthStoreBase* getAuthStore() override
    {
        return authStore.get();
    }
};

struct ScriptChannelProvider : QtTubePlugin::ChannelProvider
{
    qjs::value get_v;
    explicit ScriptChannelProvider(qjs::value&& get_v) : get_v(get_v) {}

    QtTubePlugin::ChannelReply* getChannel(
        const QString& channelId, const std::any& tabData, const std::any& continuationData) override
    {
        return makeReply<QtTubePlugin::ChannelData>(get_v, channelId, anyValue(tabData), anyValue(continuationData));
    }
};

struct ScriptChannelSubscriptionProvider : QtTubePlugin::ChannelSubscriptionProvider
{
    qjs::value setNotificationPreference_v;
    qjs::value subscribe_v;
    qjs::value unsubscribe_v;

    ScriptChannelSubscriptionProvider(
        qjs::value&& setNotificationPreference_v, qjs::value&& subscribe_v, qjs::value&& unsubscribe_v)
        : setNotificationPreference_v(std::move(setNotificationPreference_v)),
        subscribe_v(std::move(subscribe_v)),
        unsubscribe_v(std::move(unsubscribe_v)) {}

    QtTubePlugin::Reply<void>* setNotificationPreference(const QString& channelId, const std::any& data) override
    {
        if (setNotificationPreference_v.is_function())
            return makeReply<void>(setNotificationPreference_v, channelId, anyValue(data));
        else
            return nullptr;
    }

    QtTubePlugin::Reply<void>* subscribe(const QString& channelId, const std::any& data) override
    {
        return makeReply<void>(subscribe_v, channelId, anyValue(data));
    }

    QtTubePlugin::Reply<void>* unsubscribe(const QString& channelId, const std::any& data) override
    {
        return makeReply<void>(unsubscribe_v, channelId, anyValue(data));
    }
};

struct ScriptHistoryProvider : QtTubePlugin::HistoryProvider
{
    qjs::value get_v;
    explicit ScriptHistoryProvider(qjs::value&& get_v) : get_v(std::move(get_v)) {}

    QtTubePlugin::BrowseReply* getHistory(const QString& query, const std::any& continuationData) override
    {
        return makeReply<QtTubePlugin::BrowseData>(get_v, query, anyValue(continuationData));
    }
};

struct ScriptHomeProvider : QtTubePlugin::HomeProvider
{
    qjs::value get_v;
    explicit ScriptHomeProvider(qjs::value&& get_v) : get_v(std::move(get_v)) {}

    QtTubePlugin::BrowseReply* getHome(const std::any& continuationData) override
    {
        return makeReply<QtTubePlugin::BrowseData>(get_v, anyValue(continuationData));
    }
};

struct ScriptLiveChatProvider : QtTubePlugin::LiveChatProvider
{
    qjs::value get_v;
    qjs::value getReplay_v;
    qjs::value sendMessage_v;

    ScriptLiveChatProvider(qjs::value&& get_v, qjs::value&& getReplay_v, qjs::value&& sendMessage_v)
        : get_v(std::move(get_v)), getReplay_v(std::move(getReplay_v)), sendMessage_v(std::move(sendMessage_v)) {}

    QtTubePlugin::LiveChatReply* getChat(const std::any& data) override
    {
        return makeReply<QtTubePlugin::LiveChat>(get_v, anyValue(data));
    }

    QtTubePlugin::LiveChatReplayReply* getChatReplay(const std::any& data, qint64 videoOffsetMs) override
    {
        if (getReplay_v.is_function())
            return makeReply<QtTubePlugin::LiveChatReplay>(getReplay_v, anyValue(data), videoOffsetMs);
        else
            return nullptr;
    }

    QtTubePlugin::Reply<void>* sendMessage(const QString& text) override
    {
        if (sendMessage_v.is_function())
            return makeReply<void>(sendMessage_v, text);
        else
            return nullptr;
    }
};

struct ScriptNotificationsProvider : QtTubePlugin::NotificationsProvider
{
    qjs::value get_v;
    explicit ScriptNotificationsProvider(qjs::value&& get_v) : get_v(std::move(get_v)) {}

    QtTubePlugin::NotificationsReply* getNotifications(const std::any& continuationData) override
    {
        return makeReply<QtTubePlugin::NotificationsData>(get_v, anyValue(continuationData));
    }
};

struct ScriptSearchProvider : QtTubePlugin::SearchProvider
{
    qjs::value get_v;
    qjs::value resolveUrlOrID_v;
    qjs::value searchFilters_v;

    ScriptSearchProvider(qjs::value&& get_v, qjs::value&& resolveUrlOrID_v, qjs::value&& searchFilters_v)
        : get_v(std::move(get_v)), resolveUrlOrID_v(std::move(resolveUrlOrID_v)), searchFilters_v(std::move(searchFilters_v)) {}

    QtTubePlugin::BrowseReply* getSearch(
        const QString& query,
        const std::unordered_map<QString, int>& activeFilters,
        const std::any& continuationData) override
    {
        return makeReply<QtTubePlugin::BrowseData>(get_v, query, activeFilters, anyValue(continuationData));
    }

    QtTubePlugin::ResolveUrlReply* resolveUrlOrID(const QString& in) override
    {
        return makeReply<QtTubePlugin::ResolveUrlData>(resolveUrlOrID_v, in);
    }

    std::vector<std::pair<QString, QStringList>> searchFilters() override
    {
        if (searchFilters_v.is_object())
            return searchFilters_v.as<std::vector<std::pair<QString, QStringList>>>();
        else
            return {};
    }
};

/*struct ScriptSettingsProvider : QtTubePlugin::SettingsProvider
{

};*/

struct ScriptSubFeedProvider : QtTubePlugin::SubFeedProvider
{
    qjs::value get_v;
    explicit ScriptSubFeedProvider(qjs::value&& get_v) : get_v(std::move(get_v)) {}

    QtTubePlugin::BrowseReply* getSubFeed(const std::any& continuationData) override
    {
        return makeReply<QtTubePlugin::BrowseData>(get_v, anyValue(continuationData));
    }
};

struct ScriptTrendingProvider : QtTubePlugin::TrendingProvider
{
    qjs::value get_v;
    explicit ScriptTrendingProvider(qjs::value&& get_v) : get_v(std::move(get_v)) {}

    QtTubePlugin::BrowseReply* getTrending(const std::any& continuationData) override
    {
        return makeReply<QtTubePlugin::BrowseData>(get_v, anyValue(continuationData));
    }
};

struct ScriptWatchProvider : QtTubePlugin::WatchProvider
{
    qjs::value continueRecommended_v;
    qjs::value createPlayer_v;
    qjs::value get_v;
    qjs::value rate_v;

    ScriptWatchProvider(
        qjs::value&& continueRecommended_v, qjs::value&& createPlayer_v, qjs::value&& get_v, qjs::value&& rate_v)
        : continueRecommended_v(std::move(continueRecommended_v)),
          createPlayer_v(std::move(createPlayer_v)),
          get_v(std::move(get_v)),
          rate_v(std::move(rate_v)) {}

    QtTubePlugin::RecommendedReply* continueRecommended(const QString& videoId, const std::any& continuationData) override
    {
        if (continueRecommended_v.is_function())
            return makeReply<QtTubePlugin::RecommendedData>(continueRecommended_v, videoId, anyValue(continuationData));
        else
            return nullptr;
    }

    QtTubePlugin::Player* createPlayer(QtTubePlugin::PlayerSettings* settings, QWidget* parent) override
    {
        return nullptr;
    }

    QtTubePlugin::VideoReply* getVideo(const QString& videoId) override
    {
        return makeReply<QtTubePlugin::VideoData>(get_v, videoId);
    }

    QtTubePlugin::Reply<void>* rate(const QString& videoId, bool like, bool removing, const std::any& data) override
    {
        if (rate_v.is_function())
            return makeReply<void>(rate_v, videoId, like, removing, anyValue(data));
        else
            return nullptr;
    }
};