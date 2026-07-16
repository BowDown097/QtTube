#include "nativepluginentry.hpp"
#include <qttube-plugin/plugininterface.h>

namespace
{
    const QString loadFailedError = QStringLiteral("Could not load plugin from %1: %2.");
    const QString metadataNotFoundError = QStringLiteral("Could not find metadata function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
    const QString newInstanceNotFoundError = QStringLiteral("Could not find initialization function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
    const QString targetVersionNotFoundError = QStringLiteral("Could not find target version function in plugin from %1. Was the plugin set up with DECLARE_QTTUBE_PLUGIN?");
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