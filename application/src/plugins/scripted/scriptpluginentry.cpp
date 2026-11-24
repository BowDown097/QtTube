#include "scriptpluginentry.hpp"
#include "mainwindow.hpp"
#include "qttubeapplication.hpp"
#include "scriptplugininterface.hpp"
#include "scriptpluginwebplayer.hpp"
#include "utils/quickjs/jsonvaluewrappers.hpp"
#include "utils/quickjs/plugin_js_traits.hpp"
#include "utils/quickjs/polyfills/fetch.hpp"
#include "utils/quickjs/polyfills/intl.hpp"
#include "utils/quickjs/polyfills/navigator.hpp"
#include "utils/quickjs/polyfills/textencoder.hpp"
#include "utils/uiutils.hpp"
#include <qttube-plugin/utils/localcache.h>
#include <quickjs-libc.h>
#include <QWebChannel>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineView>

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

QtTubePlugin::PluginMetadata ScriptPluginEntry::createMetadata(const qjs::value& metadata)
{
    return {
        .name = QJSUtils::getStringStrict<QString>(
            metadata["name"], "Plugin name is not a string"),
        .version = QJSUtils::getStringStrict<QString>(
            metadata["version"], "Plugin version is not a string"),
        .description = QJSUtils::getStringStrict<QString>(
            metadata["description"], "Plugin description is not a string"),
        .image = QJSUtils::unwrapObjectProperty<QString>(metadata, "image"),
        .author = QJSUtils::unwrapObjectProperty<QString>(metadata, "author"),
        .url = QJSUtils::unwrapObjectProperty<QString>(metadata, "url"),
        .channelUrlTemplate = QJSUtils::getStringStrict<QString>(
            metadata["channelUrlTemplate"], "Channel URL template is not a string"),
        .videoUrlTemplate = QJSUtils::getStringStrict<QString>(
            metadata["videoUrlTemplate"], "Video URL template is not a string")
    };
}

void ScriptPluginEntry::setupContext(qjs::context& context, const QDir& dir)
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
    setupLocalCache(mod);

    Intl::registerFor(context);
    jsfetch::registerFor(context);
    Navigator::registerFor(context);
    TextEncoder::registerFor(context);
    registerEnumsFor(context);
    setupWebEngine(context);

    qjs::value globalThis = context.global();
    globalThis["devicePixelRatio"] = qApp->devicePixelRatio();
    globalThis["__scriptDir"] = dir.absolutePath();
}

void ScriptPluginEntry::setupLocalCache(qjs::module& mod)
{
    mod.register_class<LocalCache>()
        .constructor<const QByteArray&>()
        .static_member<&LocalCache::instance>()
        .static_member<&LocalCache::hash>()
        .member<&LocalCache::getName>()
        .member<&LocalCache::setMaxSeconds>()
        .member<&LocalCache::setMaxSize>()
        .member<&LocalCache::clear>()
        .member<&LocalCache::insert>()
        .member<&LocalCache::possiblyStaleValue>()
        .member<&LocalCache::value>();
}

void ScriptPluginEntry::setupWebEngine(qjs::context& ctx)
{
    using HttpCacheType = QWebEngineProfile::HttpCacheType;
    using InjectionPoint = QWebEngineScript::InjectionPoint;
    using PersistentCookiesPolicy = QWebEngineProfile::PersistentCookiesPolicy;
    using RawForm = QNetworkCookie::RawForm;
    using SameSite = QNetworkCookie::SameSite;

    qjs::enum_registrar<HttpCacheType>(ctx)
        .value<HttpCacheType::MemoryHttpCache>()
        .value<HttpCacheType::DiskHttpCache>()
        .value<HttpCacheType::NoCache>();

    qjs::enum_registrar<InjectionPoint>(ctx)
        .value<InjectionPoint::Deferred>()
        .value<InjectionPoint::DocumentReady>()
        .value<InjectionPoint::DocumentCreation>();

    qjs::enum_registrar<PersistentCookiesPolicy>(ctx)
        .value<PersistentCookiesPolicy::NoPersistentCookies>()
        .value<PersistentCookiesPolicy::AllowPersistentCookies>()
        .value<PersistentCookiesPolicy::ForcePersistentCookies>();

    qjs::enum_registrar<RawForm>(ctx)
        .value<RawForm::NameAndValueOnly>()
        .value<RawForm::Full>();

    qjs::enum_registrar<SameSite>(ctx)
        .value<SameSite::Default>()
        .value<SameSite::None>()
        .value<SameSite::Lax>()
        .value<SameSite::Strict>();

    qjs::class_registrar<QtTubePlugin::PlayerSettings>(ctx)
        .constructor<>()
        .member<&QtTubePlugin::PlayerSettings::h264Only>()
        .member<&QtTubePlugin::PlayerSettings::preferredQuality>()
        .member<&QtTubePlugin::PlayerSettings::preferredVolume>()
        .member<&QtTubePlugin::PlayerSettings::qualityFromPlayer>()
        .member<&QtTubePlugin::PlayerSettings::vaapi>()
        .member<&QtTubePlugin::PlayerSettings::volumeFromPlayer>()
        .static_member<&QtTubePlugin::PlayerSettings::qualityAsValue>();

    qjs::class_registrar<QNetworkCookie>(ctx)
        .constructor<const QByteArray&, const QByteArray&>()
        .member<&QNetworkCookie::domain>()
        .member<&QNetworkCookie::isHttpOnly>()
        .member<&QNetworkCookie::isSecure>()
        .member<&QNetworkCookie::isSessionCookie>()
        .member<&QNetworkCookie::name>()
        .member<&QNetworkCookie::normalize>()
        .member<&QNetworkCookie::path>()
        .member<&QNetworkCookie::sameSitePolicy>()
        .member<&QNetworkCookie::setDomain>()
        .member<&QNetworkCookie::setHttpOnly>()
        .member<&QNetworkCookie::setName>()
        .member<&QNetworkCookie::setPath>()
        .member<&QNetworkCookie::setSameSitePolicy>()
        .member<&QNetworkCookie::setSecure>()
        .member<&QNetworkCookie::setValue>()
        .member<&QNetworkCookie::toRawForm>()
        .member<&QNetworkCookie::value>();

    qjs::class_registrar<QWebChannel>(ctx)
        .function<true>([](QWebChannel* channel, const QString& id) {
            auto parent = static_cast<ScriptPluginWebPlayer*>(channel->parent());
            if (auto it = parent->webChannelObjects.find(id); it != parent->webChannelObjects.end())
            {
                channel->deregisterObject(it->second.get());
                parent->webChannelObjects.erase(it);
            }
        }, "deregisterObject")
        .function<true>([](QWebChannel* channel, const QString& id, const qjs::value& value) {
            auto* parent = static_cast<ScriptPluginWebPlayer*>(channel->parent());
            if (parent->webChannelObjects.contains(id))
                return;

            std::unique_ptr<QObject> wrapper;
            if (value.is_object())
                wrapper = std::make_unique<QJsonObjectWrapper>(value);
            else if (value.is_array())
                wrapper = std::make_unique<QJsonArrayWrapper>(value);
            else
                wrapper = std::make_unique<QJsonValueWrapper>(value);

            auto [it, _] = parent->webChannelObjects.emplace(id, std::move(wrapper));
            channel->registerObject(id, it->second.get());
        }, "registerObject");

    qjs::class_registrar<QWebEngineCookieStore>(ctx)
        .member<&QWebEngineCookieStore::deleteAllCookies>()
        .member<&QWebEngineCookieStore::deleteSessionCookies>()
        .member<&QWebEngineCookieStore::loadAllCookies>()
        .function<true>([](QWebEngineCookieStore* store, const std::shared_ptr<QNetworkCookie>& cookie) {
            return store->deleteCookie(*cookie);
        }, "deleteCookie")
        .function<true>([](QWebEngineCookieStore* store, const std::shared_ptr<QNetworkCookie>& cookie) {
            return store->setCookie(*cookie);
        }, "setCookie");

    qjs::class_registrar<QWebEngineProfile>(ctx)
        .member<&QWebEngineProfile::cachePath>()
        .member<&QWebEngineProfile::clearAllVisitedLinks>()
        .member<&QWebEngineProfile::clearHttpCache>()
        .member<&QWebEngineProfile::clearVisitedLinks>()
        .member<&QWebEngineProfile::cookieStore>()
        .member<&QWebEngineProfile::httpAcceptLanguage>()
        .member<&QWebEngineProfile::httpCacheMaximumSize>()
        .member<&QWebEngineProfile::httpCacheType>()
        .member<&QWebEngineProfile::httpUserAgent>()
        .member<&QWebEngineProfile::persistentCookiesPolicy>()
        .member<&QWebEngineProfile::persistentStoragePath>()
        .member<&QWebEngineProfile::setCachePath>()
        .member<&QWebEngineProfile::setHttpAcceptLanguage>()
        .member<&QWebEngineProfile::setHttpCacheMaximumSize>()
        .member<&QWebEngineProfile::setHttpCacheType>()
        .member<&QWebEngineProfile::setHttpUserAgent>()
        .member<&QWebEngineProfile::setPersistentCookiesPolicy>()
        .member<&QWebEngineProfile::setPersistentStoragePath>();

    qjs::class_registrar<QWebEnginePage>(ctx)
        .function<true>([](QWebEnginePage* page, const QString& scriptSource) {
            page->runJavaScript(scriptSource);
        }, "runJavaScript")
        .member<&QWebEnginePage::isLoading>()
        .member<&QWebEnginePage::isVisible>()
        .member<qOverload<const QUrl&>(&QWebEnginePage::load)>()
        .member<&QWebEnginePage::profile>()
        .member<&QWebEnginePage::setVisible>()
        .member<&QWebEnginePage::title>()
        .member<&QWebEnginePage::toHtml>()
        .member<&QWebEnginePage::toPlainText>()
        .member<&QWebEnginePage::url>()
        .member<&QWebEnginePage::webChannel>();

    qjs::class_registrar<QWebEngineView>(ctx)
        .member<qOverload<const QUrl&>(&QWebEngineView::load)>()
        .member<&QWebEngineView::page>()
        .member<&QWebEngineView::setContent>()
        .member<&QWebEngineView::setHtml>()
        .member<&QWebEngineView::setUrl>()
        .member<&QWebEngineView::setZoomFactor>()
        .member<&QWebEngineView::url>()
        .member<&QWebEngineView::zoomFactor>();
}
