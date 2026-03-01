#include "pluginentry.h"
#include "qttubeapplication.h"
#include <QLibrary>
#include <QMessageBox>

constexpr QLatin1String LoadAbortedError("The operation was aborted.");
constexpr QLatin1String LoadFailedError("Could not load plugin from %1: %2.");
constexpr QLatin1String MalformedMetadataError("Metadata for plugin from %1 is malformed.");
constexpr QLatin1String MetadataNotFoundError("Could not find metadata function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String NameConflictError("The name of the plugin from %1 (%2) conflicts with that of the plugin from %3 (%4).");
constexpr QLatin1String NewInstanceNotFoundError("Could not find initialization function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
constexpr QLatin1String TargetVersionMismatchWarning("The target version of the plugin from %1 (%2) does not match this version of " QTTUBE_APP_NAME " (" QTTUBE_VERSION_NAME "). This plugin may not work. Load this plugin anyway?");
constexpr QLatin1String TargetVersionNotFoundError("Could not find target version function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");

struct QLibraryDeleter
{
    void operator()(QLibrary* p) const
    {
        p->unload();
        p->deleteLater();
    }
};

PluginEntry::PluginEntry(QFileInfo&& info)
    : fileInfo(std::move(info))
{
    if (QLibrary::isLibrary(fileInfo.filePath()))
        loadAsNative();
    else if (fileInfo.suffix() == "js")
        loadAsScript();
    initialize();
}

void PluginEntry::checkMetadata()
{
    if (metadata.name.isEmpty() || metadata.version.isEmpty())
        throw PluginLoadException(MalformedMetadataError.arg(fileInfo.fileName()));

    if (PluginEntry* entry = qtTubeApp->plugins().findPlugin(metadata.name))
    {
        throw PluginLoadException(NameConflictError.arg(
            fileInfo.fileName(), metadata.name,
            entry->fileInfo.fileName(), entry->metadata.name));
    }
}

void PluginEntry::checkTargetVersion(std::string_view targetVersion)
{
    if (targetVersion != QTTUBE_VERSION_NAME)
    {
        QMessageBox::StandardButton button = QMessageBox::warning(
            nullptr, "Plugin Warning",
            TargetVersionMismatchWarning.arg(fileInfo.fileName(), targetVersion),
            QMessageBox::Yes | QMessageBox::No);
        if (button != QMessageBox::Yes)
            throw PluginLoadException(LoadAbortedError);
    }
}

void PluginEntry::initialize()
{
    interface->init();

    if (settings)
        settings->init();

    if (authStore)
    {
        authStore->init();
        authStore->restoreFromActive();
    }
}

bool PluginEntry::isPluginFile(const QFileInfo& info)
{
    return QLibrary::isLibrary(info.fileName()) || info.suffix() == "js";
}

bool PluginEntry::isPluginFile(const QString& fileName)
{
    return QLibrary::isLibrary(fileName) || fileName.endsWith(".js");
}

void PluginEntry::loadAsNative()
{
    std::unique_ptr<QLibrary, QLibraryDeleter> handle(new QLibrary(fileInfo.absoluteFilePath()));
    handle->setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);
    if (!handle->load())
        throw PluginLoadException(LoadFailedError.arg(fileInfo.fileName(), handle->errorString()));

    // check target version
    if (auto targetVersionFunc = QtTubePluginTargetVersionFunc(handle->resolve("targetVersion")))
        checkTargetVersion(targetVersionFunc());
    else
        throw PluginLoadException(TargetVersionNotFoundError.arg(fileInfo.fileName()));

    // put in and simply validate metadata, check name for conflict with already loaded plugin
    if (auto metadataFunc = QtTubePluginMetadataFunc(handle->resolve("metadata")))
        metadata = metadataFunc();
    else
        throw PluginLoadException(MetadataNotFoundError.arg(fileInfo.fileName()));
    checkMetadata();

    // put in interface
    if (auto newInstanceFunc = QtTubePluginNewInstanceFunc(handle->resolve("newInstance")))
        interface.reset(newInstanceFunc());
    else
        throw PluginLoadException(NewInstanceNotFoundError.arg(fileInfo.fileName()));

    // put in optional components
    if (auto authFunc = QtTubePluginAuthFunc(handle->resolve("auth")))
        authStore = authFunc();
    playerFunc = QtTubePluginPlayerFunc(handle->resolve("player"));
    if (auto settingsFunc = QtTubePluginSettingsFunc(handle->resolve("settings")))
        settings = settingsFunc();
}

void PluginEntry::loadAsScript()
{

}
