#include "scriptplugininterface.hpp"
#include "scriptpluginproviders.hpp"

struct BoundMethod { qjs::value v; bool required; };
struct MethodInput { const char* name; bool required; };

constexpr MethodInput operator""_opt(const char* str, size_t) { return { str, false }; }
constexpr MethodInput operator""_req(const char* str, size_t) { return { str, true }; }

template<typename Provider>
void registerProvider(
    QtTubePlugin::ProviderRegistry& reg,
    qjs::value& moduleNamespace,
    const char* providerName,
    std::same_as<MethodInput> auto... methodNames)
{
    qjs::value provider = moduleNamespace[providerName];
    if (!JS_IsObject(provider.v))
        return;

    auto methods = std::make_tuple(BoundMethod { provider[methodNames.name], methodNames.required }...);

    bool ok = std::apply([](auto&... m) {
        return (... && (!m.required || JS_IsFunction(m.v.ctx, m.v.v)));
    }, methods);
    if (!ok)
        return;

    std::apply([&](auto&... m) { reg.emplace<Provider>(std::move(m.v)...); }, methods);
}

void ScriptPluginInterface::init()
{
    if (qjs::value init_v = moduleNamespace["init"]; JS_IsFunction(init_v.ctx, init_v.v))
        init_v.invoke_then([] {});
}

void ScriptPluginInterface::registerProviders(QtTubePlugin::ProviderRegistry& reg)
{
    qjs::value authp = moduleNamespace["authenticationProvider"];
    if (JS_IsObject(authp.v))
    {
        qjs::value autho = moduleNamespace["auth"];
        qjs::value getActiveAccount = authp["getActiveAccount"];
        if (JS_IsObject(autho.v) && JS_IsFunction(getActiveAccount.ctx, getActiveAccount.v))
        {
            reg.emplace<ScriptAuthenticationProvider>(
                std::make_unique<ScriptPluginAuthStore>(pluginName, std::move(autho)),
                std::move(getActiveAccount));
        }
    }

    registerProvider<ScriptChannelProvider>(reg, moduleNamespace, "channelProvider", "getChannel"_req);
    registerProvider<ScriptChannelSubscriptionProvider>(
        reg, moduleNamespace, "channelSubscriptionProvider",
        "setNotificationPreference"_opt, "subscribe"_req, "unsubscribe"_req);
    registerProvider<ScriptHistoryProvider>(reg, moduleNamespace, "historyProvider", "getHistory"_req);
    registerProvider<ScriptHomeProvider>(reg, moduleNamespace, "homeProvider", "getHome"_req);
    registerProvider<ScriptLiveChatProvider>(
        reg, moduleNamespace, "liveChatProvider",
        "getChat"_req, "getChatReplay"_opt, "sendMessage"_opt);
    registerProvider<ScriptNotificationsProvider>(
        reg, moduleNamespace, "notificationsProvider",
        "getNotifications"_req);
    registerProvider<ScriptSearchProvider>(
        reg, moduleNamespace, "searchProvider",
        "getSearch"_req, "resolveUrlOrID"_req, "searchFilters"_opt);
    registerProvider<ScriptSubFeedProvider>(reg, moduleNamespace, "subFeedProvider", "getSubFeed"_req);
    registerProvider<ScriptTrendingProvider>(reg, moduleNamespace, "trendingProvider", "getTrending"_req);
    registerProvider<ScriptWatchProvider>(
        reg, moduleNamespace, "watchProvider",
        "continueRecommended"_opt, "createPlayer"_opt, "getVideo"_req, "rate"_opt);
}