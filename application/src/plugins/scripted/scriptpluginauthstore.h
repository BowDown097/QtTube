#pragma once
#include <qttube-plugin/components/auth/authstore.h>
#include <qttube-plugin/components/auth/webauthroutine.h>
#include <quickjs++/value.h>

struct ScriptPluginAuthUser : QtTubePlugin::AuthUser, std::vector<std::pair<QByteArray, QVariant>>
{
    explicit ScriptPluginAuthUser(const qjs::value& val);
    ScriptPluginAuthUser(bool active, const QString& avatar, const QString& id, const QString& username, const QString& handle = {})
        : AuthUser(active, avatar, id, username, handle) {}
    qjs::value toValue(JSContext* ctx) const;
};

struct ScriptPluginAuthRoutine : QtTubePlugin::WebAuthRoutine
{
    using QtTubePlugin::WebAuthRoutine::WebAuthRoutine;
    void onNewCookie(const QByteArray& name, const QByteArray& value) override;
    void onNewHeader(const QByteArray& name, const QByteArray& value) override;
    void start() override;
};

struct ScriptPluginAuthStore : QtTubePlugin::AuthStore<ScriptPluginAuthUser, ScriptPluginAuthRoutine>
{
    qjs::value authObject;

    explicit ScriptPluginAuthStore(qjs::value authObject_);
    ScriptPluginAuthUser createUser(
        const QtTubePlugin::InitialAccountData& data, const ScriptPluginAuthRoutine* routine) override;
    void init() override;
    void restoreFromActive() override;
    void save() override;
    void unauthenticate() override;
};