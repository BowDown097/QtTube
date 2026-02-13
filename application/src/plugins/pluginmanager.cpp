#include "pluginmanager.h"
#include "qttubeapplication.h"
#include <QDirIterator>
#include <QMessageBox>

constexpr QLatin1String EmplaceError("Failed to add plugin to the internal plugin list. How did this happen???");
constexpr QLatin1String LoadAbortedError("The operation was aborted.");
constexpr QLatin1String LoadFailedError("Could not load plugin from %1: %2.");
constexpr QLatin1String MalformedMetadataError("Metadata for plugin from %1 is malformed.");
constexpr QLatin1String MetadataNotFoundError("Could not find metadata function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String NameConflictError("The name of the plugin from %1 (%2) conflicts with that of the plugin from %3 (%4).");
constexpr QLatin1String NewInstanceNotFoundError("Could not find initialization function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String TargetVersionMismatchWarning("The target version of the plugin from %1 (%2) does not match this version of " QTTUBE_APP_NAME " (" QTTUBE_VERSION_NAME "). This plugin may not work. Load this plugin anyway?");
constexpr QLatin1String TargetVersionNotFoundError("Could not find target version function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");

PluginData* PluginManager::activePlugin()
{
    if (QCommandLineParser& parser = qtTubeApp->commandLineParser(); parser.isSet("use-plugin"))
        return findPlugin(parser.value("use-plugin"));

    auto it = std::ranges::find_if(m_loadedPlugins, [](const auto& p) { return p.second.active; });
    if (it != m_loadedPlugins.end())
        return &it->second;
    else
        return nullptr;
}

void PluginManager::checkPluginMetadata(const PluginData& plugin)
{
    QString pluginName = plugin.metadata.name;
    QString pluginVersion = plugin.metadata.version;

    if (pluginName.isEmpty() || pluginVersion.isEmpty())
        throw PluginLoadException(MalformedMetadataError.arg(plugin.fileInfo.fileName()));

    if (auto it = m_loadedPlugins.find(pluginName); it != m_loadedPlugins.end())
    {
        throw PluginLoadException(NameConflictError.arg(
            plugin.fileInfo.fileName(), pluginName,
            it->second.fileInfo.fileName(), it->second.metadata.name));
    }
}

void PluginManager::checkPluginTargetVersion(const PluginData& plugin)
{
    if (auto targetVersionFunc = QtTubePluginTargetVersionFunc(plugin.handle->resolve("targetVersion")))
    {
        if (const char* targetVersion = targetVersionFunc(); strcmp(targetVersion, QTTUBE_VERSION_NAME) != 0)
        {
            QMessageBox::StandardButton button = QMessageBox::warning(
                nullptr, "Plugin Warning",
                TargetVersionMismatchWarning.arg(plugin.fileInfo.fileName(), targetVersion),
                QMessageBox::Yes | QMessageBox::No);
            if (button != QMessageBox::Yes)
                throw PluginLoadException(LoadAbortedError);
        }
    }
    else
    {
        throw PluginLoadException(TargetVersionNotFoundError.arg(plugin.fileInfo.fileName()));
    }
}

void PluginManager::checkUpdate(const QString& name, const QFileInfo& updateFile)
{
    std::shared_ptr<PluginBrowser> browser(new PluginBrowser, [](PluginBrowser* p) { p->deleteLater(); });
    QSettings settings(updateFile.filePath(), QSettings::IniFormat);

    QString defaultBranch = settings.value("defaultBranch").toString();
    bool isNightly = settings.value("isNightly").toBool();
    QString repoName = settings.value("repoName").toString();
    QDateTime updatedAt = settings.value("updatedAt").toDateTime();

    if (isNightly)
        browser->getNightlyBuild(nullptr, repoName, defaultBranch);
    else
        browser->getReleaseData(nullptr, repoName, defaultBranch);

    QObject::connect(browser.get(), &PluginBrowser::gotReleaseData,
    [this, browser, name, updatedAt](BasePluginEntry*, ReleaseData data) {
        if (data.asset && data.asset->updatedAt > updatedAt)
        {
            auto [it, _] = m_updatablePlugins.emplace(name, std::move(data));
            emit foundUpdate(it->first, it->second);
        }
    });
}

bool PluginManager::containsPlugin(const QString& name)
{
    return m_loadedPlugins.contains(name);
}

PluginData* PluginManager::findPlugin(const QString& name)
{
    if (auto it = m_loadedPlugins.find(name); it != m_loadedPlugins.end())
        return &it->second;
    else
        return nullptr;
}

bool PluginManager::hasAuthenticated() const
{
    const PluginData* plugin = qtTubeApp->plugins().activePlugin();
    return plugin && plugin->auth && !plugin->auth->isEmpty();
}

const QList<QDir>& PluginManager::libraryLoadDirs()
{
    static const QList<QDir> libraryLoadDirs = []() -> QList<QDir> {
        if (!qtTubeApp->isPortableBuild())
        {
            return {
                FS::joinPaths(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation), "plugin-libs"),
                FS::joinPaths(qApp->applicationDirPath(), "plugin-libs")
            };
        }
        else
        {
            return { FS::joinPaths(qApp->applicationDirPath(), "plugin-libs") };
        }
    }();
    return libraryLoadDirs;
}

PluginData* PluginManager::loadAndInitPlugin(PluginData&& plugin)
{
    plugin.interface->init();

    if (plugin.settings)
        plugin.settings->init();

    if (plugin.auth)
    {
        plugin.auth->init();
        plugin.auth->restoreFromActive();
    }

    if (auto res = m_loadedPlugins.emplace(plugin.metadata.name, std::move(plugin)); res.second)
        return &res.first->second;
    else
        throw PluginLoadException(EmplaceError);
}

PluginData* PluginManager::loadAndInitPlugin(const QFileInfo& fileInfo)
{
    return loadAndInitPlugin(openPlugin(fileInfo));
}

QList<PluginData*> PluginManager::loadedPlugins()
{
    QList<PluginData*> out;
    out.reserve(m_loadedPlugins.size());
    for (auto& [_, lp] : m_loadedPlugins)
        out.append(&lp);
    return out;
}

PluginData PluginManager::openPlugin(const QFileInfo& fileInfo)
{
    // create PluginData with what we have so far, then try to load the handle
    PluginData plugin = {
        .fileInfo = fileInfo,
        .handle = QLibraryPtr(new QLibrary(fileInfo.absoluteFilePath()))
    };

    plugin.handle->setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);
    if (!plugin.handle->load())
        throw PluginLoadException(LoadFailedError.arg(plugin.fileInfo.fileName(), plugin.handle->errorString()));

    // check target version
    checkPluginTargetVersion(plugin);

    // put in and simply validate metadata, check name for conflict with already loaded plugin
    if (auto metadataFunc = QtTubePluginMetadataFunc(plugin.handle->resolve("metadata")))
        plugin.metadata = metadataFunc();
    else
        throw PluginLoadException(MetadataNotFoundError.arg(plugin.fileInfo.fileName()));

    checkPluginMetadata(plugin);

    // put in interface
    if (auto newInstanceFunc = QtTubePluginNewInstanceFunc(plugin.handle->resolve("newInstance")))
        plugin.interface.reset(newInstanceFunc());
    else
        throw PluginLoadException(NewInstanceNotFoundError.arg(plugin.fileInfo.fileName()));

    // put in optional components
    if (auto authFunc = QtTubePluginAuthFunc(plugin.handle->resolve("auth")))
        plugin.auth = authFunc();
    plugin.playerFunc = QtTubePluginPlayerFunc(plugin.handle->resolve("player"));
    if (auto settingsFunc = QtTubePluginSettingsFunc(plugin.handle->resolve("settings")))
        plugin.settings = settingsFunc();

    return plugin;
}

const QList<QDir>& PluginManager::pluginLoadDirs()
{
    static const QList<QDir> pluginLoadDirs = []() -> QList<QDir> {
        if (!qtTubeApp->isPortableBuild())
        {
            return {
                FS::joinPaths(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation), "plugins"),
                FS::joinPaths(qApp->applicationDirPath(), "plugins")
            };
        }
        else
        {
            return { FS::joinPaths(qApp->applicationDirPath(), "plugins") };
        }
    }();
    return pluginLoadDirs;
}

void PluginManager::reloadPlugins()
{
    QString activePluginName;
    if (const PluginData* plugin = activePlugin())
        activePluginName = plugin->fileInfo.fileName();
    else if (!qtTubeApp->settings().activePlugin.isEmpty())
        activePluginName = qtTubeApp->settings().activePlugin;

    m_foundPluginFile = false;
    m_loadedPlugins.clear();

    QList<QFileInfo> pluginsToLoad;
    auto storeInfo = [&](QFileInfo&& info) {
        QFileInfo& stored = pluginsToLoad.emplaceBack(std::move(info));
        if (activePluginName.isEmpty())
            activePluginName = stored.fileName();
    };

    for (const QDir& dir : pluginLoadDirs())
    {
        constexpr QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
        for (QDirIterator it(dir.path(), filters); it.hasNext();)
        {
            QFileInfo info(it.next());
            if (info.isDir())
            {
                for (QDirIterator it2(info.filePath(), filters); it2.hasNext();)
                {
                    if (QFileInfo info2(it2.next()); info2.fileName() == "update.ini")
                    {
                        QString pluginName = info.fileName();
                        if (!m_updatablePlugins.contains(pluginName))
                            checkUpdate(pluginName, info2);
                    }
                    else if (QLibrary::isLibrary(info2.filePath()))
                    {
                        storeInfo(std::move(info2));
                    }
                }
            }
            else if (QLibrary::isLibrary(info.filePath()))
            {
                storeInfo(std::move(info));
            }
        }
    }

    m_foundPluginFile = !pluginsToLoad.isEmpty();

    for (const QFileInfo& fileInfo : pluginsToLoad)
    {
        try
        {
            PluginData plugin = openPlugin(fileInfo);
            if (fileInfo.fileName() == activePluginName)
                plugin.active = true;
            loadAndInitPlugin(std::move(plugin));
        }
        catch (const PluginLoadException& ex)
        {
            QMessageBox::critical(nullptr, "Plugin Error", ex.message());
        }
    }
}
