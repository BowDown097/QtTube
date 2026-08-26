#include "scriptpluginentry.hpp"
#include "mainwindow.hpp"
#include "qttubeapplication.hpp"
#include "scriptplugininterface.hpp"
#include "utils/quickjs/plugin_js_traits.hpp"
#include "utils/quickjs/polyfills/fetch.hpp"
#include "utils/quickjs/polyfills/intl.hpp"
#include "utils/quickjs/polyfills/navigator.hpp"
#include "utils/quickjs/qjsutils.hpp"
#include "utils/uiutils.hpp"
#include <quickjs-libc.h>

QtTubePlugin::PluginMetadata createMetadata(const qjs::value& metadata)
{
    return {
        .name = QJSUtils::getStringStrict<QString>(
            metadata["name"], "Plugin name is not a string"),
        .version = QJSUtils::getStringStrict<QString>(
            metadata["version"], "Plugin version is not a string"),
        .description = QJSUtils::getStringStrict<QString>(
            metadata["description"], "Plugin description is not a string"),
        .image = QJSUtils::unwrapObjectProperty<QString>(
            metadata.ctx, metadata.v, "image"),
        .author = QJSUtils::unwrapObjectProperty<QString>(
            metadata.ctx, metadata.v, "author"),
        .url = QJSUtils::unwrapObjectProperty<QString>(
            metadata.ctx, metadata.v, "url"),
        .channelUrlTemplate = QJSUtils::getStringStrict<QString>(
            metadata["channelUrlTemplate"], "Channel URL template is not a string"),
        .videoUrlTemplate = QJSUtils::getStringStrict<QString>(
            metadata["videoUrlTemplate"], "Video URL template is not a string")
    };
}

void setupContext(qjs::context& context, const QDir& dir)
{
    js_std_add_helpers(context.ctx, 0, nullptr);
    js_init_module_std(context.ctx, "std");
    js_init_module_os(context.ctx, "os");

    context.module_loader = [&](std::string_view filename) {
        std::filesystem::path path(filename);
        if (!std::filesystem::exists(path))
        {
            UIUtils::getMainWindow()->reportJsException(
                QStringLiteral("ReferenceError: Module file not found: %1").arg(
                    QString::fromUtf8(filename.data(), filename.size())));
        }

        return qjs::context::module_data(qjs::detail::to_uri(filename), qjs::detail::read_file(path));
    };

    context.on_unhandled_promise_rejection = [](qjs::value val) {
        if (val.is_error())
            UIUtils::getMainWindow()->reportJsException(QJSUtils::generateErrorString(val));
    };

    qjs::module& mod = context.add_module("QtTube");
    mod.add("relativeTimeString", &UIUtils::relativeTimeString);

    Intl::registerFor(context);
    jsfetch::registerFor(context);
    Navigator::registerFor(context);
    registerEnumsFor(context);

    context.global()["__scriptDir"] = dir.absolutePath();
}

void ScriptPluginEntry::initialize()
{
    std::unique_ptr<qjs::context> context = std::make_unique<qjs::context>(qtTubeApp->jsRuntime());

    try
    {
        setupContext(*context, fileInfo.dir());

        qjs::value funcVal = context->eval_file(
            fileInfo.filePath().toUtf8(),
            JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_STRICT | JS_EVAL_FLAG_COMPILE_ONLY);
        if (JS_VALUE_GET_TAG(funcVal.v) != JS_TAG_MODULE)
            throw PluginLoadException("Code compiled to non-module object.");

        JSModuleDef* module = static_cast<JSModuleDef*>(JS_VALUE_GET_PTR(funcVal.v));
        if (!module)
            throw PluginLoadException("Code somehow compiled to an unresolvable module.");

        qjs::value evalResult = context->new_value(JS_EvalFunction(context->ctx, funcVal.release()));
        JSPromiseStateEnum state = JS_PromiseState(evalResult.ctx, evalResult.v);
        if (state == JS_PROMISE_FULFILLED)
        {
            qjs::value moduleNamespace(context->ctx, JS_GetModuleNamespace(context->ctx, module));
            qjs::value metadataProperty = moduleNamespace["metadata"];

            PluginEntry::checkTargetVersion(metadataProperty["targetVersion"].as<std::string_view>());
            metadata = createMetadata(metadataProperty);
            PluginEntry::checkMetadata();

            interface = std::make_unique<ScriptPluginInterface>(
                metadata.name, std::move(moduleNamespace), std::move(context));
        }
        else if (state == JS_PROMISE_REJECTED)
        {
            throw PluginLoadException(QJSUtils::generateErrorString(
                context->new_value(JS_PromiseResult(evalResult.ctx, evalResult.v))));
        }
        else
        {
            throw PluginLoadException(
                QStringLiteral("Evalution of module code returned an unexpected result. "
                               "Promise state: %1").arg(state));
        }
    }
    catch (const qjs::exception& ex)
    {
        throw PluginLoadException(QJSUtils::generateErrorString(ex));
    }

    PluginEntry::initialize();
}