#include "pluginbuilddownloader.h"
#include "qttubeapplication.h"
#include "qttube-plugin/utils/httprequest.h"
#include <QTemporaryFile>

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

QString writeFile(const QString& name, const QByteArray& data, std::optional<QFileInfo>* pluginFile = nullptr)
{
    QFile file(name);
    if (!file.open(QFile::WriteOnly))
        return "Could not open " + name + " for writing: " + file.errorString();

    file.write(data);
    if (pluginFile)
        pluginFile->emplace(file);
    return QString();
}

PluginBuildDownloader::PluginBuildDownloader(QString pluginName, ReleaseData data, QObject* parent)
    : m_data(std::move(data)),
      m_pluginName(std::move(pluginName)),
      m_tempFile(new QTemporaryFile(this))
{
    if (!m_tempFile->open())
    {
        emit failed("Could not open plugin file for writing.");
        return;
    }

    HttpReply* reply = HttpRequest().writingToIODevice(m_tempFile).get(m_data.asset->downloadUrl);
    connect(reply, &HttpReply::downloadProgress, this,
        std::bind_front(&PluginBuildDownloader::progress, this, m_data.asset->name));
    connect(reply, &HttpReply::finished, this, &PluginBuildDownloader::downloadFinished);
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
    m_tempFile->seek(0);

    const QDir& libsDir = qtTubeApp->plugins().libraryLoadDirs().front();
    libsDir.mkpath(".");

    QDir pluginDir(qtTubeApp->plugins().pluginLoadDirs().front().filePath(m_pluginName));
    pluginDir.mkpath(".");

    QString fileError;
    std::optional<QFileInfo> pluginFile;

    if (m_data.asset->name.endsWith(".zip"))
    {
        QZipReader zipReader(m_tempFile);
        const QList<QZipReader::FileInfo> files = zipReader.fileInfoList();

        for (const QZipReader::FileInfo& info : files)
        {
            if (!info.isFile || !PluginEntry::isPluginFile(info.filePath))
                continue;

            if (info.filePath.startsWith("libs/"))
                fileError = writeFile(libsDir.filePath(info.filePath.section('/', -1)), zipReader.fileData(info.filePath));
            else if (!pluginFile && !info.filePath.contains('/'))
                fileError = writeFile(pluginDir.filePath(info.filePath), zipReader.fileData(info.filePath), &pluginFile);
        }
    }
    else
    {
        fileError = writeFile(pluginDir.filePath(m_data.asset->name), m_tempFile->readAll(), &pluginFile);
    }

    if (!fileError.isEmpty())
    {
        pluginDir.removeRecursively();
        emit failed(fileError);
        return;
    }

    if (!pluginFile)
    {
        pluginDir.removeRecursively();
        emit failed("No plugin file found.");
        return;
    }

    try
    {
        createUpdateIni(pluginDir.filePath("update.ini"));
        emit finished(qtTubeApp->plugins().registerPlugin(std::move(pluginFile.value())));
    }
    catch (const PluginLoadException& ex)
    {
        pluginDir.removeRecursively();
        emit failed(ex.message());
    }
}
