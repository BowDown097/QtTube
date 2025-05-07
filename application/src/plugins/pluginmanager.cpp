#include "pluginmanager.h"
#include <QDirIterator>
#include <QMessageBox>

constexpr QLatin1String LoadFailedError("Could not load plugin from %1.");
constexpr QLatin1String MetadataNotFoundError("Could not find metadata function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String NameConflictError("The name of the plugin from %1 conflicts with that of the plugin from %2 (%3).");
constexpr QLatin1String NewInstanceNotFoundError("Could not find initialization function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String SettingsNotFoundError("Could not find settings function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String TargetVersionMismatchWarning("The target version of the plugin from %1 (%2) does not match this version of " QTTUBE_APP_NAME " (" QTTUBE_VERSION_NAME "). This plugin may not work. Load this plugin anyway?");
constexpr QLatin1String TargetVersionNotFoundError("Could not find target version function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");

bool PluginManager::checkPluginTargetVersion(const QFileInfo& fileInfo)
{
    std::unique_ptr<QLibrary, QLibraryDeleter> handle(new QLibrary(fileInfo.absoluteFilePath()));
    if (!handle->load())
    {
        QMessageBox::critical(nullptr, "Plugin Error", LoadFailedError.arg(fileInfo.fileName()));
        return false;
    }

    QFunctionPointer versionFuncPtr = handle->resolve("targetVersion");
    if (!versionFuncPtr)
    {
        QMessageBox::critical(nullptr, "Plugin Error", TargetVersionNotFoundError.arg(fileInfo.fileName()));
        return false;
    }

    const char* targetVersion = QtTubePluginVersionFunc(versionFuncPtr)();
    if (strcmp(targetVersion, QTTUBE_VERSION_NAME) != 0)
    {
        QMessageBox::StandardButton button = QMessageBox::warning(nullptr, "Plugin Warning",
            TargetVersionMismatchWarning.arg(fileInfo.fileName(), targetVersion), QMessageBox::Yes | QMessageBox::No);
        return button == QMessageBox::Yes;
    }

    return true;
}

const PluginData* PluginManager::findPlugin(const QString& name) const
{
    auto it = m_plugins.find(name);
    if (it != m_plugins.end())
        return &it->second;
    else
        return nullptr;
}

std::optional<PluginData> PluginManager::loadPlugin(const QFileInfo& fileInfo)
{
    if (!checkPluginTargetVersion(fileInfo))
        return {};

    std::unique_ptr<QLibrary, QLibraryDeleter> handle(new QLibrary(fileInfo.absoluteFilePath()));
    handle->setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);
    if (!handle->load())
    {
        QMessageBox::critical(nullptr, "Plugin Error", LoadFailedError.arg(fileInfo.fileName()));
        return {};
    }

    auto metadataFunc = QtTubePluginMetadataFunc(handle->resolve("metadata"));
    if (!metadataFunc)
    {
        QMessageBox::critical(nullptr, "Plugin Error", MetadataNotFoundError.arg(fileInfo.fileName()));
        return {};
    }

    auto newInstanceFunc = QtTubePluginNewInstanceFunc(handle->resolve("newInstance"));
    if (!newInstanceFunc)
    {
        QMessageBox::critical(nullptr, "Plugin Error", NewInstanceNotFoundError.arg(fileInfo.fileName()));
        return {};
    }

    auto settingsFunc = QtTubePluginSettingsFunc(handle->resolve("settings"));
    if (!settingsFunc)
    {
        QMessageBox::critical(nullptr, "Plugin Error", SettingsNotFoundError.arg(fileInfo.fileName()));
        return {};
    }

    return PluginData {
        .fileInfo = fileInfo,
        .handle = std::move(handle),
        .interface = std::unique_ptr<QtTube::PluginInterface>(newInstanceFunc()),
        .metadata = metadataFunc(),
        .settings = settingsFunc(),
    };
}

QList<const PluginData*> PluginManager::plugins() const
{
    QList<const PluginData*> out;
    out.reserve(m_plugins.size());
    for (const auto& [_, lp] : m_plugins)
        out.append(&lp);
    return out;
}

void PluginManager::reloadPlugins()
{
    m_plugins.clear();

    QList<QFileInfo> pluginsToLoad;
    for (QDirIterator it("plugins", QDir::Files); it.hasNext();)
        if (QFileInfo fileInfo(it.next()); QLibrary::isLibrary(fileInfo.absoluteFilePath()))
            pluginsToLoad.append(fileInfo);

    for (const QFileInfo& fileInfo : pluginsToLoad)
    {
        if (std::optional<PluginData> plugin = loadPlugin(fileInfo))
        {
            if (auto it = m_plugins.find(plugin->metadata->name); it == m_plugins.end())
            {
                plugin->interface->init();
                m_plugins.emplace(plugin->metadata->name, std::move(plugin.value()));
            }
            else
            {
                QMessageBox::critical(nullptr, "Plugin Error",
                    NameConflictError.arg(fileInfo.fileName(), it->second.fileInfo.fileName(), plugin->metadata->name));
            }
        }
    }
}
