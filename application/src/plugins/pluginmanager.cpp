#include "pluginmanager.h"
#include <QApplication>
#include <QDirIterator>
#include <QMessageBox>

constexpr QLatin1String LoadFailedError("Could not load plugin from %1: %2.");
constexpr QLatin1String MetadataNotFoundError("Could not find metadata function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String NameConflictError("The name of the plugin from %1 (%2) conflicts with that of the plugin from %3 (%4).");
constexpr QLatin1String NewInstanceNotFoundError("Could not find initialization function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String TargetVersionMismatchWarning("The target version of the plugin from %1 (%2) does not match this version of " QTTUBE_APP_NAME " (" QTTUBE_VERSION_NAME "). This plugin may not work. Load this plugin anyway?");
constexpr QLatin1String TargetVersionNotFoundError("Could not find target version function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");

template<typename R>
R pluginError(const QString& error, R&& returnValue)
{
    QMessageBox::critical(nullptr, "Plugin Error", error);
    return returnValue;
}

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

bool PluginManager::checkPluginTargetVersion(const QFileInfo& fileInfo)
{
    std::unique_ptr<QLibrary, QLibraryDeleter> handle(new QLibrary(fileInfo.absoluteFilePath()));
    if (!handle->load())
        return pluginError(LoadFailedError.arg(fileInfo.fileName(), handle->errorString()), false);

    QFunctionPointer versionFuncPtr = handle->resolve("targetVersion");
    if (!versionFuncPtr)
        return pluginError(TargetVersionNotFoundError.arg(fileInfo.fileName()), false);

    const char* targetVersion = QtTubePluginVersionFunc(versionFuncPtr)();
    if (strcmp(targetVersion, QTTUBE_VERSION_NAME) != 0)
    {
        QMessageBox::StandardButton button = QMessageBox::warning(nullptr, "Plugin Warning",
            TargetVersionMismatchWarning.arg(fileInfo.fileName(), targetVersion), QMessageBox::Yes | QMessageBox::No);
        return button == QMessageBox::Yes;
    }

    return true;
}

PluginData* PluginManager::findPlugin(const QString& name)
{
    if (auto it = m_loadedPlugins.find(name); it != m_loadedPlugins.end())
        return &it->second;
    else
        return nullptr;
}

std::optional<PluginData> PluginManager::loadPlugin(const QFileInfo& fileInfo)
{
    // check version
    if (!checkPluginTargetVersion(fileInfo))
        return {};

    // create PluginData with what we have so far
    PluginData data;
    data.fileInfo = fileInfo;
    data.handle = std::unique_ptr<QLibrary, QLibraryDeleter>(new QLibrary(fileInfo.absoluteFilePath()));
    data.handle->setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);

    // load handle, get required instance and metadata components
    if (!data.handle->load())
        return pluginError(LoadFailedError.arg(fileInfo.fileName(), data.handle->errorString()), std::nullopt);

    if (auto newInstanceFunc = QtTubePluginNewInstanceFunc(data.handle->resolve("newInstance")))
        data.interface.reset(newInstanceFunc());
    else
        return pluginError(NewInstanceNotFoundError.arg(fileInfo.fileName()), std::nullopt);

    if (auto metadataFunc = QtTubePluginMetadataFunc(data.handle->resolve("metadata")))
        data.metadata = metadataFunc();
    else
        return pluginError(MetadataNotFoundError.arg(fileInfo.fileName()), std::nullopt);

    // optional components
    if (auto authFunc = QtTubePluginAuthFunc(data.handle->resolve("auth")))
        data.auth = authFunc();
    if (auto settingsFunc = QtTubePluginSettingsFunc(data.handle->resolve("settings")))
        data.settings = settingsFunc();

    return data;
}

const QList<PluginData*> PluginManager::loadedPlugins()
{
    QList<PluginData*> out;
    out.reserve(m_loadedPlugins.size());
    for (auto& [_, lp] : m_loadedPlugins)
        out.append(&lp);
    return out;
}

void PluginManager::reloadPlugins()
{
    QString presentlyActivePluginName;
    if (const PluginData* plugin = activePlugin())
        presentlyActivePluginName = plugin->fileInfo.fileName();

    m_loadedPlugins.clear();

    QList<QFileInfo> pluginsToLoad;
    for (QDirIterator it(qApp->applicationDirPath() + QDir::separator() + "plugins", QDir::Files); it.hasNext();)
    {
        QFileInfo fileInfo(it.next());
        if (presentlyActivePluginName.isEmpty())
            presentlyActivePluginName = fileInfo.fileName();
        if (QLibrary::isLibrary(fileInfo.absoluteFilePath()))
            pluginsToLoad.append(fileInfo);
    }

    for (const QFileInfo& fileInfo : pluginsToLoad)
    {
        if (std::optional<PluginData> plugin = loadPlugin(fileInfo))
        {
            if (auto it = m_loadedPlugins.find(plugin->metadata->name); it == m_loadedPlugins.end())
            {
                if (fileInfo.fileName() == presentlyActivePluginName)
                    plugin->active = true;
                if (plugin->auth)
                    plugin->auth->init();
                if (plugin->settings)
                    plugin->settings->init();
                plugin->interface->init();
                m_loadedPlugins.emplace(plugin->metadata->name, std::move(plugin.value()));
            }
            else
            {
                QMessageBox::critical(nullptr, "Plugin Error", NameConflictError.arg(
                    fileInfo.fileName(), plugin->metadata->name,
                    it->second.fileInfo.fileName(), it->second.metadata->name));
            }
        }
    }
}
