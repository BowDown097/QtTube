#include "pluginentry.h"
#include "qttubeapplication.h"
#include "qttube-plugin/plugininterface.h"
#include "scripted/scriptpluginentry.h"
#include <QMessageBox>

namespace
{
    const QString loadAbortedError = QStringLiteral("The operation was aborted.");
    const QString loadFailedError = QStringLiteral("Could not load plugin from %1: %2.");
    const QString malformedMetadataError = QStringLiteral("Metadata for plugin from %1 is malformed.");
    const QString metadataNotFoundError = QStringLiteral("Could not find metadata function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
    const QString nameConflictError = QStringLiteral("The name of the plugin from %1 (%2) conflicts with that of the plugin from %3 (%4).");
    const QString newInstanceNotFoundError = QStringLiteral("Could not find initialization function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
    const QString targetVersionMismatchWarning = QStringLiteral("The target version of the plugin from %1 (%2) does not match this version of " QTTUBE_APP_NAME " (" QTTUBE_VERSION_NAME "). This plugin may not work. Load this plugin anyway?");
    const QString targetVersionNotFoundError = QStringLiteral("Could not find target version function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
}

PluginEntry::PluginEntry(QFileInfo&& fileInfo_) : fileInfo(std::move(fileInfo_)) {}
PluginEntry::~PluginEntry() = default;
PluginEntry::PluginEntry(PluginEntry&&) = default;
PluginEntry& PluginEntry::operator=(PluginEntry&&) = default;

std::unique_ptr<PluginEntry> PluginEntry::create(QFileInfo&& fileInfo)
{
    if (QLibrary::isLibrary(fileInfo.filePath()))
        return std::make_unique<NativePluginEntry>(std::move(fileInfo));
    else if (fileInfo.suffix() == "js")
        return std::make_unique<ScriptPluginEntry>(std::move(fileInfo));
    return nullptr;
}

void PluginEntry::checkMetadata()
{
    const QString name = metadata.name;
    const QString version = metadata.version;

    if (name.isEmpty() || version.isEmpty())
        throw PluginLoadException(malformedMetadataError.arg(fileInfo.fileName()));

    if (PluginEntry* entry = qtTubeApp->plugins().findPlugin(name))
    {
        throw PluginLoadException(nameConflictError.arg(
            fileInfo.fileName(), name,
            entry->fileInfo.fileName(), entry->metadata.name));
    }
}

void PluginEntry::checkTargetVersion(std::string_view targetVersion)
{
    if (targetVersion != QTTUBE_VERSION_NAME)
    {
        QMessageBox::StandardButton button = QMessageBox::warning(
            nullptr, "Plugin Warning",
            targetVersionMismatchWarning.arg(fileInfo.fileName(), targetVersion.data()),
            QMessageBox::Yes | QMessageBox::No);
        if (button != QMessageBox::Yes)
            throw PluginLoadException(loadAbortedError);
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

bool PluginEntry::isPluginFile(const QString& fileName)
{
    return QLibrary::isLibrary(fileName) || fileName.endsWith(".js");
}

NativePluginEntry::NativePluginEntry(QFileInfo&& fileInfo_)
    : PluginEntry(std::move(fileInfo_)), m_handle(fileInfo.absoluteFilePath())
{
    m_handle.setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);
}

void NativePluginEntry::initialize()
{
    if (!m_handle.load())
        throw PluginLoadException(loadFailedError.arg(fileInfo.fileName(), m_handle.errorString()));

    // check target version
    if (auto targetVersionFunc = QtTubePluginTargetVersionFunc(m_handle.resolve("targetVersion")))
        checkTargetVersion(targetVersionFunc());
    else
        throw PluginLoadException(targetVersionNotFoundError.arg(fileInfo.fileName()));

    // put in and simply validate metadata, check name for conflict with already loaded plugin
    if (auto metadataFunc = QtTubePluginMetadataFunc(m_handle.resolve("metadata")))
        metadata = metadataFunc();
    else
        throw PluginLoadException(metadataNotFoundError.arg(fileInfo.fileName()));
    checkMetadata();

    // put in interface
    if (auto newInstanceFunc = QtTubePluginNewInstanceFunc(m_handle.resolve("newInstance")))
        interface.reset(newInstanceFunc());
    else
        throw PluginLoadException(newInstanceNotFoundError.arg(fileInfo.fileName()));

    // put in optional components
    if (auto authFunc = QtTubePluginAuthFunc(m_handle.resolve("auth")))
        authStore = authFunc();
    playerFunc = QtTubePluginPlayerFunc(m_handle.resolve("player"));
    if (auto settingsFunc = QtTubePluginSettingsFunc(m_handle.resolve("settings")))
        settings = settingsFunc();

    PluginEntry::initialize();
}

void NativePluginEntry::unload()
{
    m_handle.unload();
}