#include "pluginentry.hpp"
#include "nativepluginentry.hpp"
#include "qttubeapplication.hpp"
#include "scripted/scriptpluginentry.hpp"
#include <QMessageBox>
#include <qttube-plugin/plugininterface.h>

namespace
{
    const QString loadAbortedError = QStringLiteral("The operation was aborted.");
    const QString malformedMetadataError = QStringLiteral("Metadata for plugin from %1 is malformed.");
    const QString nameConflictError = QStringLiteral("The name of the plugin from %1 (%2) conflicts with that of the plugin from %3 (%4).");
    const QString targetVersionMismatchWarning = QStringLiteral("The target version of the plugin from %1 (%2) does not match this version of " QTTUBE_APP_NAME " (" QTTUBE_VERSION_NAME "). This plugin may not work. Load this plugin anyway?");
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
    if (interface)
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