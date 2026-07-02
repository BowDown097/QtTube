#include "pluginbuilddownloader.hpp"
#include "qttubeapplication.hpp"
#include <qttube-plugin/utils/httprequest.h>
#include <quazip.h>
#include <quazipfile.h>

void writeFile(QIODevice& device, const QString& outName, std::optional<QFileInfo>* pluginFile)
{
    QFile file(outName);
    if (!file.open(QIODevice::WriteOnly))
        throw PluginLoadException("Could not open " % outName % " for writing: " % file.errorString());

    file.write(device.readAll());
    if (pluginFile)
        pluginFile->emplace(file);
}

void extractZipEntry(
    const QuaZipFileInfo64& info, QuaZip& zip,
    const QString& outName, std::optional<QFileInfo>* pluginFile = {})
{
    QuaZipFile file(&zip);
    if (!file.open(QIODevice::ReadOnly))
        throw PluginLoadException("Could not open " % info.name % " for reading: " % file.errorString());
    writeFile(file, outName, pluginFile);
}

void PluginBuildDownloader::createUpdateIni(const QString& path)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("defaultBranch", m_data.defaultBranch);
    settings.setValue("isNightly", m_data.isNightly);
    settings.setValue("repoName", m_data.fullName);
    settings.setValue("updatedAt", m_data.asset->updatedAt);
}

void PluginBuildDownloader::downloadFinished(const HttpReply& reply)
{
    if (!reply.isSuccessful())
    {
        emit failed(reply.errorString());
        return;
    }

    try
    {
        m_tempFile.seek(0);

        const QDir& libsDir = qtTubeApp->plugins().libraryLoadDirs().front();
        libsDir.mkpath(".");
        m_pluginDir.mkpath(".");

        std::optional<QFileInfo> pluginFile;
        if (m_data.asset->name.endsWith(".zip"))
        {
            QuaZip zip(&m_tempFile);
            if (!zip.open(QuaZip::mdUnzip))
                throw PluginLoadException("Failed to unzip " + m_data.asset->name);

            QuaZipFileInfo64 info;
            for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
            {
                if (zip.getCurrentFileInfo(&info) && PluginEntry::isPluginFile(info.name))
                {
                    if (info.name.startsWith("libs/"))
                        extractZipEntry(info, zip, libsDir.filePath(info.name.section('/', -1)));
                    else if (!pluginFile && !info.name.contains('/'))
                        extractZipEntry(info, zip, m_pluginDir.filePath(info.name), &pluginFile);
                }
            }
        }
        else
        {
            writeFile(m_tempFile, m_pluginDir.filePath(m_data.asset->name), &pluginFile);
        }

        if (!pluginFile.has_value())
            throw PluginLoadException("No plugin file found.");

        createUpdateIni(m_pluginDir.filePath("update.ini"));
        emit finished(qtTubeApp->plugins().registerPlugin(std::move(pluginFile.value())));
    }
    catch (const PluginLoadException& ex)
    {
        m_pluginDir.removeRecursively();
        emit failed(ex.message());
    }
}

void PluginBuildDownloader::start()
{
    if (!m_tempFile.open())
    {
        emit failed("Could not open plugin file for writing.");
        return;
    }

    HttpReply* reply = HttpRequest().writingToIODevice(&m_tempFile).get(m_data.asset->downloadUrl);
    connect(reply, &HttpReply::downloadProgress, this,
        std::bind_front(&PluginBuildDownloader::progress, this, m_data.asset->name));
    connect(reply, &HttpReply::finished, this, &PluginBuildDownloader::downloadFinished);
}