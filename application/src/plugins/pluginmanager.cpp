#include "pluginbrowser.hpp"
#include "pluginmanager.hpp"
#include "qttubeapplication.hpp"
#include <QDirIterator>
#include <QMessageBox>
#include <qttube-plugin/providers/providertypes.h>

namespace
{
    const QString emplaceError = QStringLiteral("Failed to add plugin to the internal plugin list. How did this happen???");
    const QString invalidFileError = QStringLiteral("Given file is not a valid plugin file.");
}

PluginEntry* PluginManager::activePlugin() const
{
    if (QCommandLineParser& parser = qtTubeApp->commandLineParser(); parser.isSet("use-plugin"))
        return findPlugin(parser.value("use-plugin"));

    auto it = std::ranges::find_if(m_loadedPlugins, [](const auto& p) { return p.second->active; });
    if (it != m_loadedPlugins.end())
        return it->second.get();
    else
        return nullptr;
}

void PluginManager::checkUpdate(const QString& name, const QFileInfo& updateFile)
{
    if (m_updatablePlugins.contains(name))
        return;

    std::shared_ptr<PluginBrowser> browser = std::make_shared<PluginBrowser>();
    QSettings settings(updateFile.filePath(), QSettings::IniFormat);

    QString defaultBranch = settings.value("defaultBranch").toString();
    bool isNightly = settings.value("isNightly").toBool();
    QString repoName = settings.value("repoName").toString();
    QDateTime updatedAt = settings.value("updatedAt").toDateTime();

    if (isNightly)
        browser->getNightlyBuild(nullptr, repoName, defaultBranch);
    else
        browser->getReleaseData(nullptr, repoName, defaultBranch);

    QObject::connect(browser.get(), &PluginBrowser::gotReleaseData, browser.get(),
    [this, browser, name, updatedAt](BasePluginEntry*, ReleaseData data) {
        if (data.asset && data.asset->updatedAt > updatedAt)
        {
            auto [it, _] = m_updatablePlugins.emplace(name, std::move(data));
            emit foundUpdate(it->first, it->second);
        }
    });
}

bool PluginManager::containsPlugin(const QString& name) const
{
    return m_loadedPlugins.contains(name);
}

PluginEntry* PluginManager::findPlugin(const QString& name) const
{
    if (auto it = m_loadedPlugins.find(name); it != m_loadedPlugins.end())
        return it->second.get();
    else
        return nullptr;
}

QList<QFileInfo> PluginManager::getPluginsToLoad(QString& activePluginName)
{
    static constexpr QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QList<QFileInfo> pluginsToLoad;

    auto handleInfo = [&](auto& self, const QString& root, QFileInfo&& info) -> void {
        if (info.isDir())
        {
            for (QDirIterator it(info.filePath(), filters); it.hasNext();)
                self(self, root, QFileInfo(it.next()));
        }
        else if (info.fileName() == "update.ini")
        {
            checkUpdate(root, info);
        }
        else if (PluginEntry::isPluginFile(info.fileName()))
        {
            QFileInfo& stored = pluginsToLoad.emplaceBack(std::move(info));
            if (activePluginName.isEmpty())
                activePluginName = stored.fileName();
        }
    };

    for (const QDir& dir : pluginLoadDirs())
    {
        for (QDirIterator it(dir.path(), filters); it.hasNext();)
        {
            QFileInfo info(it.next());
            handleInfo(handleInfo, info.fileName(), std::move(info));
        }
    }

    return pluginsToLoad;
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

const QList<PluginEntry*> PluginManager::loadedPlugins() const
{
    QList<PluginEntry*> out;
    out.reserve(m_loadedPlugins.size());
    for (auto& [_, lp] : m_loadedPlugins)
        out.append(lp.get());
    return out;
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

PluginEntry* PluginManager::registerPlugin(QFileInfo&& fileInfo)
{
    std::unique_ptr<PluginEntry> plugin = PluginEntry::create(std::move(fileInfo));
    if (plugin)
        plugin->initialize();
    else
        throw PluginLoadException(invalidFileError);

    if (auto res = m_loadedPlugins.emplace(plugin->metadata.name, std::move(plugin)); res.second)
        return res.first->second.get();
    else
        throw PluginLoadException(emplaceError);
}

void PluginManager::reloadPlugins()
{
    m_loadedPlugins.clear();

    QString activePluginName;
    if (PluginEntry* plugin = activePlugin())
        activePluginName = plugin->fileInfo.fileName();
    else if (!qtTubeApp->settings().activePlugin.isEmpty())
        activePluginName = qtTubeApp->settings().activePlugin;

    QList<QFileInfo> pluginsToLoad = getPluginsToLoad(activePluginName);
    m_foundPluginFile = !pluginsToLoad.isEmpty();

    for (QFileInfo& fileInfo : pluginsToLoad)
    {
        try
        {
            PluginEntry* plugin = registerPlugin(std::move(fileInfo));
            if (plugin->fileInfo.fileName() == activePluginName)
                plugin->active = true;
        }
        catch (const PluginLoadException& ex)
        {
            QMessageBox::critical(nullptr, "Plugin Error (" + fileInfo.fileName() + ')', ex.message());
        }
    }
}
