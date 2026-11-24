#include "scriptpluginwebplayer.hpp"
#include "qttube-plugin/components/reply.h"
#include "utils/quickjs/qjsutils.hpp"
#include <QDir>
#include <quickjs++/context.h>

ScriptPluginWebPlayer::ScriptPluginWebPlayer(
    QtTubePlugin::PlayerSettings* settings, const qjs::value& player_v, QWidget* parent)
    : QtTubePlugin::WebPlayer(settings, parent), player_v(player_v)
{
    try
    {
        qjs::context& ctx = qjs::context::get(player_v.ctx);
        QDir moduleDir(ctx.global()["__scriptDir"].as<QString>());

        if (qjs::value setup_v = player_v["setup"]; setup_v.is_function())
            setup_v.invoke(makeSelfObject());

        if (qjs::value scriptFiles_v = player_v["scriptFiles"]; scriptFiles_v.is_array())
        {
            const QList<qjs::value> scriptFiles = scriptFiles_v.as<QList<qjs::value>>();
            for (const qjs::value& scriptFile : scriptFiles)
            {
                if (scriptFile.is_object())
                {
                    loadScriptFile(
                        moduleDir.absoluteFilePath(scriptFile["path"].as<QString>()),
                        scriptFile["injectionPoint"].as<QWebEngineScript::InjectionPoint>(),
                        scriptFile["worldId"].as<quint32>());
                }
                else if (scriptFile.is_string())
                {
                    loadScriptFile(moduleDir.absoluteFilePath(scriptFile.as<QString>()));
                }
            }
        }

        if (qjs::value styleFiles_v = player_v["styleFiles"]; styleFiles_v.is_array())
        {
            const QStringList styleFiles = styleFiles_v.as<QStringList>();
            for (const QString& styleFile : styleFiles)
                loadStyleFile(moduleDir.absoluteFilePath(styleFile));
        }
    }
    catch (const qjs::exception& ex)
    {
        throw QtTubePlugin::Exception(QJSUtils::generateErrorString(ex));
    }
}

qjs::value ScriptPluginWebPlayer::makeSelfObject() const
{
    qjs::value v(player_v.ctx, JS_NewObject(player_v.ctx));
    v["channel"] = m_channel;
    v["settings"] = settings();
    v["view"] = m_view;
    return v;
}

void ScriptPluginWebPlayer::play(const QString& videoId, int progress)
{
    try
    {
        if (qjs::value play_v = player_v["play"]; play_v.is_function())
            play_v.invoke(makeSelfObject(), videoId, progress);
    }
    catch (const qjs::exception& ex)
    {
        throw QtTubePlugin::Exception(QJSUtils::generateErrorString(ex));
    }
}

void ScriptPluginWebPlayer::seek(int progress)
{
    try
    {
        if (qjs::value seek_v = player_v["seek"]; seek_v.is_function())
            seek_v.invoke(makeSelfObject(), progress);
    }
    catch (const qjs::exception& ex)
    {
        throw QtTubePlugin::Exception(QJSUtils::generateErrorString(ex));
    }
}