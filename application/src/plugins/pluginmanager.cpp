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
    auto it = std::ranges::find_if(m_loadedPlugins, [](const std::pair<const QString, PluginData>& p) {
        return p.second.active;
    });
    if (it != m_loadedPlugins.end())
        return &it->second;
    else
        return nullptr;
}

void PluginManager::checkPluginMetadata(const PluginData& data)
{
    QString pluginName = data.metadata.name;
    QString pluginVersion = data.metadata.version;

    if (pluginName.isEmpty() || pluginVersion.isEmpty())
        throw PluginLoadException(MalformedMetadataError.arg(data.fileInfo.fileName()));

    if (auto it = m_loadedPlugins.find(pluginName); it != m_loadedPlugins.end())
    {
        throw PluginLoadException(NameConflictError.arg(
            data.fileInfo.fileName(), pluginName,
            it->second.fileInfo.fileName(), it->second.metadata.name));
    }
}

void PluginManager::checkPluginTargetVersion(const PluginData& data)
{
    if (auto targetVersionFunc = QtTubePluginTargetVersionFunc(data.handle->resolve("targetVersion")))
    {
        if (const char* targetVersion = targetVersionFunc(); strcmp(targetVersion, QTTUBE_VERSION_NAME) != 0)
        {
            QMessageBox::StandardButton button = QMessageBox::warning(
                nullptr, "Plugin Warning",
                TargetVersionMismatchWarning.arg(data.fileInfo.fileName(), targetVersion),
                QMessageBox::Yes | QMessageBox::No);
            if (button != QMessageBox::Yes)
                throw PluginLoadException(LoadAbortedError);
        }
    }
    else
    {
        throw PluginLoadException(TargetVersionNotFoundError.arg(data.fileInfo.fileName()));
    }
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

const QStringList& PluginManager::libraryLoadDirs()
{
    static const QStringList libraryLoadDirs = {
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QDir::separator() + "plugin-libs",
        qApp->applicationDirPath() + QDir::separator() + "plugin-libs"
    };
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

const QList<PluginData*> PluginManager::loadedPlugins()
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
    PluginData data = {
        .fileInfo = fileInfo,
        .handle = QLibraryPtr(new QLibrary(fileInfo.absoluteFilePath()))
    };

    data.handle->setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);
    if (!data.handle->load())
        throw PluginLoadException(LoadFailedError.arg(data.fileInfo.fileName(), data.handle->errorString()));

    // check target version
    checkPluginTargetVersion(data);

    // put in and simply validate metadata, check name for conflict with already loaded plugin
    if (auto metadataFunc = QtTubePluginMetadataFunc(data.handle->resolve("metadata")))
        data.metadata = metadataFunc();
    else
        throw PluginLoadException(MetadataNotFoundError.arg(data.fileInfo.fileName()));

    checkPluginMetadata(data);

    // put in interface
    if (auto newInstanceFunc = QtTubePluginNewInstanceFunc(data.handle->resolve("newInstance")))
        data.interface.reset(newInstanceFunc());
    else
        throw PluginLoadException(NewInstanceNotFoundError.arg(data.fileInfo.fileName()));

    // put in optional components
    if (auto authFunc = QtTubePluginAuthFunc(data.handle->resolve("auth")))
        data.auth = authFunc();
    data.playerFunc = QtTubePluginPlayerFunc(data.handle->resolve("player"));
    if (auto settingsFunc = QtTubePluginSettingsFunc(data.handle->resolve("settings")))
        data.settings = settingsFunc();

    return data;
}

const QStringList& PluginManager::pluginLoadDirs()
{
    static const QStringList pluginLoadDirs = {
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QDir::separator() + "plugins",
        qApp->applicationDirPath() + QDir::separator() + "plugins"
    };
    return pluginLoadDirs;
}

void PluginManager::reloadPlugins()
{
    QString presentlyActivePluginName;
    if (const PluginData* plugin = activePlugin())
        presentlyActivePluginName = plugin->fileInfo.fileName();
    else if (!qtTubeApp->settings().activePlugin.isEmpty())
        presentlyActivePluginName = qtTubeApp->settings().activePlugin;

    m_loadedPlugins.clear();

    QList<QFileInfo> pluginsToLoad;
    for (const QString& pluginLoadDir : pluginLoadDirs())
    {
        for (QDirIterator it(pluginLoadDir, QDir::Files); it.hasNext();)
        {
            if (QFileInfo fileInfo(it.next()); QLibrary::isLibrary(fileInfo.absoluteFilePath()))
            {
                pluginsToLoad.append(fileInfo);
                if (presentlyActivePluginName.isEmpty())
                    presentlyActivePluginName = fileInfo.fileName();
            }
        }
    }

    for (const QFileInfo& fileInfo : pluginsToLoad)
    {
        try
        {
            PluginData plugin = openPlugin(fileInfo);
            if (fileInfo.fileName() == presentlyActivePluginName)
                plugin.active = true;
            loadAndInitPlugin(std::move(plugin));
        }
        catch (const PluginLoadException& ex)
        {
            QMessageBox::critical(nullptr, "Plugin Error", ex.message());
        }
    }
}
