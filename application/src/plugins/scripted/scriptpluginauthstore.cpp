#include "scriptpluginauthstore.h"
#include "mainwindow.h"
#include "utils/quickjs/qt_js_traits.h"
#include "utils/uiutils.h"
#include <quickjs++/value.h>
#include <unordered_set>

static const std::unordered_set<QByteArrayView> builtinKeys = {
    "active",
    "avatar",
    "handle",
    "id",
    "username"
};

ScriptPluginAuthUser::ScriptPluginAuthUser(const qjs::value& val)
{
    active   = val["active"].as<bool>();
    avatar   = val["avatar"].as<QString>();
    handle   = val["handle"].as<QString>();
    id       = val["id"].as<QString>();
    username = val["username"].as<QString>();

    for (const auto& [key, value] : val.properties<QByteArray, QVariant>())
        if (!builtinKeys.contains(key))
            emplace_back(key, value);
}

qjs::value ScriptPluginAuthUser::toValue(JSContext* ctx) const
{
    qjs::value obj(ctx, JS_NewObject(ctx));

    obj["active"]   = active;
    obj["avatar"]   = avatar;
    obj["handle"]   = handle;
    obj["id"]       = id;
    obj["username"] = username;

    for (const auto& [key, value] : (*this))
        obj[key] = value;

    return obj;
}

void ScriptPluginAuthRoutine::onNewCookie(const QByteArray& name, const QByteArray& value)
{
    const qjs::value& authObject = static_cast<ScriptPluginAuthStore*>(m_authStore)->authObject;
    if (qjs::value fn = authObject["onNewCookie"]; JS_IsFunction(fn.ctx, fn.v))
        fn.invoke_then([] {}, name, value);
}

void ScriptPluginAuthRoutine::onNewHeader(const QByteArray& name, const QByteArray& value)
{
    const qjs::value& authObject = static_cast<ScriptPluginAuthStore*>(m_authStore)->authObject;
    if (qjs::value fn = authObject["onNewHeader"]; JS_IsFunction(fn.ctx, fn.v))
        fn.invoke_then([] {}, name, value);
}

void ScriptPluginAuthRoutine::start()
{
    const qjs::value& authObject = static_cast<ScriptPluginAuthStore*>(m_authStore)->authObject;

    if (qjs::value urlVal = authObject["url"]; JS_IsString(urlVal.v))
    {
        setUrl(urlVal.as<QString>());
    }
    else
    {
        UIUtils::getMainWindow()->reportJsException("TypeError: auth object needs url");
        return;
    }

    if (qjs::value loginBtnVal = authObject["loginButton"]; JS_IsString(loginBtnVal.v))
        setLoginButton(loginBtnVal.as<QString>());

    QList<QtTubePlugin::SearchCookie> searchCookies;
    QList<QByteArray> searchHeaders;

    if (qjs::value searchCookiesVal = authObject["searchCookies"]; JS_IsArray(searchCookiesVal.v))
    {
        for (const qjs::value& cookieVal : searchCookiesVal.as<std::vector<qjs::value>>())
        {
            QtTubePlugin::SearchCookie cookie;

            if (JS_IsString(cookieVal.v))
            {
                cookie.name = cookieVal.as<QByteArray>();
            }
            else if (JS_IsObject(cookieVal.v))
            {
                if (qjs::value name = cookieVal["name"]; JS_IsString(name.v))
                {
                    cookie.name = name.as<QByteArray>();
                }
                else
                {
                    UIUtils::getMainWindow()->reportJsException("TypeError: name not provided for search cookie");
                    return;
                }

                if (qjs::value domain = cookieVal["domain"]; JS_IsString(domain.v))
                    cookie.domain = domain.as<QByteArray>();

                if (qjs::value path = cookieVal["path"]; JS_IsString(path.v))
                    cookie.path = path.as<QByteArray>();
            }
            else
            {
                UIUtils::getMainWindow()->reportJsException("TypeError: search cookie expected to be string or object");
                return;
            }

            searchCookies.append(std::move(cookie));
        }
    }

    if (qjs::value searchHeadersVal = authObject["searchHeaders"]; JS_IsArray(searchHeadersVal.v))
        searchHeaders = searchHeadersVal.as<QList<QByteArray>>();

    if (searchCookies.isEmpty() && searchHeaders.isEmpty())
    {
        UIUtils::getMainWindow()->reportJsException("TypeError: auth object needs searchCookies or searchHeaders");
        return;
    }

    setSearchCookies(searchCookies);
    setSearchHeaders(searchHeaders);
    QtTubePlugin::WebAuthRoutine::start();
}

ScriptPluginAuthStore::ScriptPluginAuthStore(qjs::value authObject_)
    : authObject(std::move(authObject_))
{
    authObject["activeLogin"] = [this] {
        if (const ScriptPluginAuthUser* active = activeLogin())
            return active->toValue(authObject.ctx);
        return qjs::value(JS_NULL);
    };

    authObject["append"] = [this](const qjs::value& value) {
        return this->append(ScriptPluginAuthUser(value));
    };

    authObject["credentials"] = [this]() {
        const QList<ScriptPluginAuthUser*> creds = credentials();
        QList<qjs::value> out;
        for (const ScriptPluginAuthUser* user : creds)
            out.append(user->toValue(authObject.ctx));
        return out;
    };
}

ScriptPluginAuthUser ScriptPluginAuthStore::createUser(
    const QtTubePlugin::InitialAccountData& data, const ScriptPluginAuthRoutine* routine)
{
    QHash<QByteArray, QByteArray> cookies = routine->searchCookies();
    QHash<QByteArray, QByteArray> headers = routine->searchHeaders();

    ScriptPluginAuthUser out(
        true,
        data.avatarUrl,
        data.channelId,
        data.displayName,
        data.handle
    );

    for (auto it = cookies.begin(); it != cookies.end(); ++it)
        out.emplace_back(it.key(), it.value());
    for (auto it = headers.begin(); it != headers.end(); ++it)
        out.emplace_back(it.key(), it.value());

    return out;
}

void ScriptPluginAuthStore::init()
{
    QSettings settings(configPath(), QSettings::IniFormat);
    const QStringList groups = settings.childGroups();

    for (const QString& group : groups)
    {
        settings.beginGroup(group);

        ScriptPluginAuthUser user(
            settings.value("active").toBool(),
            settings.value("avatar").toString(),
            group, // id
            settings.value("username").toString(),
            settings.value("handle").toString()
        );

        const QStringList keys = settings.childKeys();
        for (const QString& key : keys)
        {
            const QByteArray utf8Key = key.toUtf8();
            if (!builtinKeys.contains(utf8Key))
                user.emplace_back(utf8Key, settings.value(key));
        }

        settings.endGroup();
        append(std::move(user));
    }
}

void ScriptPluginAuthStore::restoreFromActive()
{
    if (qjs::value fn = authObject["restore"]; JS_IsFunction(fn.ctx, fn.v))
        if (const ScriptPluginAuthUser* active = activeLogin())
            fn.invoke_then([] {}, active->toValue(fn.ctx));
}

void ScriptPluginAuthStore::save()
{
    QSettings settings(configPath(), QSettings::IniFormat);
    const QList<ScriptPluginAuthUser*> users = this->credentials();

    for (const ScriptPluginAuthUser* user : users)
    {
        settings.beginGroup(user->id);

        settings.setValue("active", user->active);
        settings.setValue("avatar", user->avatar);
        settings.setValue("handle", user->handle);
        settings.setValue("username", user->username);
        for (const auto& [key, value] : (*user))
            settings.setValue(QString::fromUtf8(key), value);

        settings.endGroup();
    }
}

void ScriptPluginAuthStore::unauthenticate()
{
    if (qjs::value fn = authObject["unauthenticate"]; JS_IsFunction(fn.ctx, fn.v))
        fn.invoke_then([] {});
}