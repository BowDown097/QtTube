#include "scriptpluginauthstore.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "utils/quickjs/qjsutils.h"
#include "utils/uiutils.h"

bool ScriptPluginAuthRoutine::onNewCookie(const QByteArray& name, const QByteArray& value)
{
    const qjs::value& authObject = static_cast<ScriptPluginAuthStore*>(m_authStore)->authObject;
    if (qjs::value fn = authObject["onNewCookie"]; JS_IsFunction(fn.ctx, fn.v))
        return fn.as<std::function<bool(QByteArray, QByteArray)>>()(name, value);
    return true;
}

bool ScriptPluginAuthRoutine::onNewHeader(const QByteArray& name, const QByteArray& value)
{
    const qjs::value& authObject = static_cast<ScriptPluginAuthStore*>(m_authStore)->authObject;
    if (qjs::value fn = authObject["onNewHeader"]; JS_IsFunction(fn.ctx, fn.v))
        return fn.as<std::function<bool(QByteArray, QByteArray)>>()(name, value);
    return true;
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

ScriptPluginAuthStore::ScriptPluginAuthStore(const QString& pluginName, qjs::value authObject_)
    : authObject(std::move(authObject_))
{
    setConfigPath(resolveConfigPath(pluginName, "auth", qtTubeApp->isPortableBuild()));

    authObject["activeLogin"] = [this] {
        if (const ScriptPluginAuthUser* active = activeLogin())
            return qjs::value(authObject.ctx, *active);
        return qjs::value(JS_NULL);
    };

    authObject["append"] = [this](const qjs::value& value) {
        return this->append(value.as<ScriptPluginAuthUser>());
    };

    authObject["credentials"] = [this]() {
        const QList<ScriptPluginAuthUser*> creds = credentials();
        QList<qjs::value> out;
        for (const ScriptPluginAuthUser* user : creds)
            out.emplaceBack(authObject.ctx, *user);
        return out;
    };
}

ScriptPluginAuthUser ScriptPluginAuthStore::createUser(
    const QtTubePlugin::InitialAccountData& data, const ScriptPluginAuthRoutine* routine)
{
    ScriptPluginAuthUser out(
        true,
        data.avatarUrl,
        data.channelId,
        data.displayName,
        data.handle
    );

    out.cookies = routine->searchCookies();
    out.headers = routine->searchHeaders();

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

        auto makeCredGroup = [&](const QString& prefix, std::unordered_map<QByteArray, QByteArray>& container) {
            settings.beginGroup(prefix);

            const QStringList keys = settings.childKeys();
            container.reserve(keys.size());
            for (const QString& key : keys)
                container.emplace(key.toUtf8(), settings.value(key).toByteArray());

            settings.endGroup();
        };

        makeCredGroup("cookies", user.cookies);
        makeCredGroup("headers", user.headers);

        settings.endGroup();
        append(std::move(user));
    }
}

void ScriptPluginAuthStore::restoreFromActive()
{
    if (qjs::value fn = authObject["restore"]; JS_IsFunction(fn.ctx, fn.v))
        if (const ScriptPluginAuthUser* active = activeLogin())
            fn.invoke_then([] {}, *active);
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

        auto saveCredGroup = [&](const QString& prefix, const std::unordered_map<QByteArray, QByteArray>& container) {
            settings.beginGroup(prefix);
            for (const auto& [key, value] : container)
                settings.setValue(QString::fromUtf8(key), value);
            settings.endGroup();
        };

        saveCredGroup("cookies", user->cookies);
        saveCredGroup("headers", user->headers);

        settings.endGroup();
    }
}

void ScriptPluginAuthStore::unauthenticate()
{
    if (qjs::value fn = authObject["unauthenticate"]; JS_IsFunction(fn.ctx, fn.v))
        fn.invoke_then([] {});
}

namespace qjs
{
    ScriptPluginAuthUser js_traits<ScriptPluginAuthUser>::unwrap(JSContext* ctx, JSValueConst val)
    {
        ScriptPluginAuthUser result(
            QJSUtils::unwrapObjectProperty<bool>(ctx, val, "active"),
            QJSUtils::unwrapObjectProperty<QString>(ctx, val, "avatar"),
            QJSUtils::unwrapObjectProperty<QString>(ctx, val, "id"),
            QJSUtils::unwrapObjectProperty<QString>(ctx, val, "username"),
            QJSUtils::unwrapObjectProperty<QString>(ctx, val, "handle")
        );

        result.cookies = QJSUtils::unwrapObjectProperty<std::unordered_map<QByteArray, QByteArray>>(ctx, val, "cookies");
        result.headers = QJSUtils::unwrapObjectProperty<std::unordered_map<QByteArray, QByteArray>>(ctx, val, "headers");

        return result;
    }

    JSValue js_traits<ScriptPluginAuthUser>::wrap(JSContext* ctx, const ScriptPluginAuthUser& val)
    {
        qjs::value obj(ctx, JS_NewObject(ctx));
        obj["active"] = val.active;
        obj["avatar"] = val.avatar;
        obj["cookies"] = val.cookies;
        obj["headers"] = val.headers;
        obj["handle"] = val.handle;
        obj["id"] = val.id;
        obj["username"] = val.username;
        return obj.release();
    }

    JSValue property_traits<QtTubePlugin::SearchCookie>::get(
        JSContext* ctx, JSValue this_obj, const QtTubePlugin::SearchCookie& key)
    {
        return property_traits<QByteArray>::get(ctx, this_obj, key.name);
    }

    void property_traits<QtTubePlugin::SearchCookie>::set(
        JSContext* ctx, JSValue this_obj, const QtTubePlugin::SearchCookie& key, JSValue val)
    {
        return property_traits<QByteArray>::set(ctx, this_obj, key.name, val);
    }
}