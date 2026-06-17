#pragma once
#include <qttube-plugin/components/auth/authstore.h>
#include <qttube-plugin/components/auth/webauthroutine.h>
#include <quickjs++/value.h>

struct ScriptPluginAuthUser : QtTubePlugin::AuthUser
{
    std::unordered_map<QByteArray, QByteArray> cookies;
    std::unordered_map<QByteArray, QByteArray> headers;

    ScriptPluginAuthUser(bool active, const QString& avatar, const QString& id, const QString& username, const QString& handle = {})
        : AuthUser(active, avatar, id, username, handle) {}
};

struct ScriptPluginAuthRoutine : QtTubePlugin::WebAuthRoutine
{
    using QtTubePlugin::WebAuthRoutine::WebAuthRoutine;
    bool onNewCookie(const QByteArray& name, const QByteArray& value) override;
    bool onNewHeader(const QByteArray& name, const QByteArray& value) override;
    void start() override;
};

struct ScriptPluginAuthStore : QtTubePlugin::AuthStore<ScriptPluginAuthUser, ScriptPluginAuthRoutine>
{
    qjs::value authObject;

    explicit ScriptPluginAuthStore(const QString& pluginName, qjs::value authObject_);
    ScriptPluginAuthUser createUser(
        const QtTubePlugin::InitialAccountData& data, const ScriptPluginAuthRoutine* routine) override;
    void init() override;
    void restoreFromActive() override;
    void save() override;
    void unauthenticate() override;
};

namespace qjs
{
    template<> struct js_traits<ScriptPluginAuthUser>
    {
        static ScriptPluginAuthUser unwrap(JSContext* ctx, JSValueConst val);
        static JSValue wrap(JSContext* ctx, const ScriptPluginAuthUser& val);
    };

    template<> struct property_traits<QtTubePlugin::SearchCookie>
    {
        static JSValue get(JSContext* ctx, JSValue this_obj, const QtTubePlugin::SearchCookie& key);
        static void set(JSContext* ctx, JSValue this_obj, const QtTubePlugin::SearchCookie& key, JSValue val);
    };
}