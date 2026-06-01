#include "scriptplugininterface.h"
#include "qttube-plugin/utils/replyutils.h"
#include "utils/quickjs/plugin_js_traits.h"
#include "utils/quickjs/qjsutils.h"

using namespace Qt::StringLiterals;

template<typename T>
inline T defaultAnyCast(const std::any& value)
{
    try { return std::any_cast<T>(value); }
    catch (const std::bad_any_cast&) { return T{}; }
}

template<typename T>
inline QtTubePlugin::Reply<T>* makeReply(qjs::value& moduleNamespace, QLatin1String key, auto&&... args)
{
    using ReplyType = QtTubePlugin::Reply<T>;
    ReplyType* reply = ReplyType::create();

    qjs::value funcValue = moduleNamespace[key];
    if (!JS_IsFunction(funcValue.ctx, funcValue.v))
    {
        QtTubePlugin::invokeQueued(reply, &ReplyType::exception, QtTubePlugin::Exception(
            QStringLiteral("TypeError: No function found with name '%1'").arg(key)));
        return reply;
    }

    try
    {
        if constexpr (std::is_void_v<T>)
        {
            funcValue.invoke_then([=] {
                QtTubePlugin::invokeQueued(reply, &ReplyType::finished);
            }, std::forward<decltype(args)>(args)...);
        }
        else
        {
            funcValue.invoke_then([=](const T& data) {
                QtTubePlugin::invokeQueued(reply, &ReplyType::finished, data);
            }, std::forward<decltype(args)>(args)...);
        }
    }
    catch (const qjs::exception& ex)
    {
        QtTubePlugin::invokeQueued(reply, &ReplyType::exception,
            QtTubePlugin::Exception(QJSUtils::generateErrorString(ex.get_value())));
    }

    return reply;
}

ScriptPluginInterface::ScriptPluginInterface(
    qjs::value&& moduleNamespace, std::unique_ptr<qjs::context>&& context)
    : m_context(std::move(context)), m_moduleNamespace(std::move(moduleNamespace))
{
    if (qjs::value auth = m_moduleNamespace["auth"]; JS_IsObject(auth.v))
        m_authStore = std::make_unique<ScriptPluginAuthStore>(std::move(auth));
}

ScriptPluginInterface::~ScriptPluginInterface()
{
    m_moduleNamespace.release();
}

QtTubePlugin::RecommendedContinuationReply* ScriptPluginInterface::continueRecommended(
    const QString& videoId, std::any continuationData)
{
    return makeReply<QtTubePlugin::RecommendedContinuationData>(
        m_moduleNamespace, "continueRecommended"_L1, videoId, defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::AccountReply* ScriptPluginInterface::getActiveAccount()
{
    return makeReply<QtTubePlugin::InitialAccountData>(m_moduleNamespace, "getActiveAccount"_L1);
}

QtTubePlugin::ChannelReply* ScriptPluginInterface::getChannel(
    const QString& channelId, std::any tabData, std::any continuationData)
{
    return makeReply<QtTubePlugin::ChannelData>(
        m_moduleNamespace, "getChannel"_L1,
        channelId, defaultAnyCast<JSValue>(tabData), defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::BrowseReply* ScriptPluginInterface::getHistory(
    const QString& query, std::any continuationData)
{
    return makeReply<QtTubePlugin::BrowseData>(
        m_moduleNamespace, "getHistory"_L1, query, defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::BrowseReply* ScriptPluginInterface::getHome(std::any continuationData)
{
    return makeReply<QtTubePlugin::BrowseData>(
        m_moduleNamespace, "getHome"_L1, defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::LiveChatReply* ScriptPluginInterface::getLiveChat(std::any data)
{
    return makeReply<QtTubePlugin::LiveChat>(
        m_moduleNamespace, "getLiveChat"_L1, defaultAnyCast<JSValue>(data));
}

QtTubePlugin::LiveChatReplayReply* ScriptPluginInterface::getLiveChatReplay(
    std::any data, qint64 videoOffsetMs)
{
    return makeReply<QtTubePlugin::LiveChatReplay>(
        m_moduleNamespace, "getLiveChatReplay"_L1, defaultAnyCast<JSValue>(data), videoOffsetMs);
}

QtTubePlugin::NotificationsReply* ScriptPluginInterface::getNotifications(std::any continuationData)
{
    return makeReply<QtTubePlugin::NotificationsData>(
        m_moduleNamespace, "getNotifications"_L1, defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::BrowseReply* ScriptPluginInterface::getSearch(
    const QString& query, const QList<std::pair<QString, int>>& activeFilters, std::any continuationData)
{
    return makeReply<QtTubePlugin::BrowseData>(
        m_moduleNamespace, "getSearch"_L1, query, activeFilters, defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::BrowseReply* ScriptPluginInterface::getSubFeed(std::any continuationData)
{
    return makeReply<QtTubePlugin::BrowseData>(
        m_moduleNamespace, "getSubFeed"_L1, defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::BrowseReply* ScriptPluginInterface::getTrending(std::any continuationData)
{
    return makeReply<QtTubePlugin::BrowseData>(
        m_moduleNamespace, "getTrending"_L1, defaultAnyCast<JSValue>(continuationData));
}

QtTubePlugin::VideoReply* ScriptPluginInterface::getVideo(const QString& videoId)
{
    return makeReply<QtTubePlugin::VideoData>(m_moduleNamespace, "getVideo"_L1, videoId);
}

void ScriptPluginInterface::init()
{
    if (qjs::value initValue = m_moduleNamespace["init"]; JS_IsFunction(initValue.ctx, initValue.v))
        initValue.invoke_then([] {});
}

QtTubePlugin::Reply<void>* ScriptPluginInterface::rate(
    const QString& videoId, bool like, bool removing, std::any data)
{
    return makeReply<void>(
        m_moduleNamespace, "rate"_L1, videoId, like, removing, defaultAnyCast<JSValue>(data));
}

QtTubePlugin::ResolveUrlReply* ScriptPluginInterface::resolveUrlOrID(const QString& in)
{
    return makeReply<QtTubePlugin::ResolveUrlData>(m_moduleNamespace, "resolveUrlOrID"_L1, in);
}

const QList<std::pair<QString, QStringList>> ScriptPluginInterface::searchFilters() const
{
    return m_moduleNamespace["searchFilters"].as<
        std::function<QList<std::pair<QString, QStringList>>()>>()();
}

QtTubePlugin::Reply<void>* ScriptPluginInterface::sendLiveChatMessage(const QString& text)
{
    return makeReply<void>(m_moduleNamespace, "sendLiveChatMessage"_L1, text);
}

QtTubePlugin::Reply<void>* ScriptPluginInterface::setNotificationPreference(std::any data)
{
    return makeReply<void>(m_moduleNamespace, "setNotificationPreference"_L1, defaultAnyCast<JSValue>(data));
}

QtTubePlugin::Reply<void>* ScriptPluginInterface::subscribe(std::any data)
{
    return makeReply<void>(m_moduleNamespace, "subscribe"_L1, defaultAnyCast<JSValue>(data));
}

QtTubePlugin::Reply<void>* ScriptPluginInterface::unsubscribe(std::any data)
{
    return makeReply<void>(m_moduleNamespace, "unsubscribe"_L1, defaultAnyCast<JSValue>(data));
}
