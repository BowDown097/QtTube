#include "pluginbuilddownloader.h"
#include "plugins/pluginbrowser.h"
#include "qttube-plugin/utils/httprequest.h"
#include "qttubeapplication.h"
#include <QBoxLayout>
#include <QDir>
#include <QLibrary>
#include <QMessageBox>
#include <QProgressBar>
#include <QTemporaryFile>

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

namespace
{
    QString bytesString(const QString& format, double bytes)
    {
        QString out;
        if (bytes < 1024)
        {
            out = format.arg(bytes, 3, 'f', 1).arg("B");
        }
        else if (bytes < 1048576)
        {
            bytes /= 1024;
            out = format.arg(bytes, 3, 'f', 1).arg("KB");
        }
        else if (bytes < 1073741824)
        {
            bytes /= 1048576;
            out = format.arg(bytes, 3, 'f', 1).arg("MB");
        }
        else
        {
            bytes /= 1073741824;
            out = format.arg(bytes, 3, 'f', 1).arg("GB");
        }

        return out;
    }

    bool writeFile(const QString& name, const QByteArray& data, std::optional<QFileInfo>* pluginFile = nullptr)
    {
        if (QFile file(name); file.open(QFile::WriteOnly))
        {
            file.write(data);
            if (pluginFile)
                pluginFile->emplace(file);
            return true;
        }
        else
        {
            QMessageBox::critical(nullptr, "Failed to Install Plugin", "Could not open " + name + " for writing.");
            return false;
        }
    }
}

PluginBuildDownloader::PluginBuildDownloader(const ReleaseData& data, QWidget* parent)
    : QWidget(parent),
      m_assetName(data.asset.name),
      m_fileName(data.asset.downloadUrl.section('/', -1)),
      m_layout(new QVBoxLayout(this)),
      m_progressBar(new QProgressBar(this)),
      m_tempFile(new QTemporaryFile(this))
{
    if (!m_tempFile->open())
    {
        QMessageBox::critical(this, "Failed to Install Plugin", "Could not open plugin file for writing.");
        deleteLater();
        return;
    }

    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(width(), 100);
    setWindowTitle("Plugin Downloader");

    m_progressBar->setFixedHeight(50);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_progressBar);

    HttpReply* reply = HttpRequest().writingToIODevice(m_tempFile).get(data.asset.downloadUrl);
    connect(reply, &HttpReply::downloadProgress, this, &PluginBuildDownloader::downloadProgress);
    connect(reply, &HttpReply::finished, this, &PluginBuildDownloader::downloadFinished);
}

void PluginBuildDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (!m_timer.isValid())
        m_timer.start();

    m_progressBar->setMaximum(bytesTotal);
    m_progressBar->setValue(bytesReceived);

    QString current = bytesString(QStringLiteral("%1 %2"), bytesReceived);
    QString speed = bytesString(QStringLiteral("%1 %2/s"), bytesReceived * 1000.0 / m_timer.elapsed());
    QString total = bytesString(QStringLiteral("%1 %2"), bytesTotal);
    m_progressBar->setFormat(QStringLiteral("%1: %2 / %3 (%4)").arg(m_assetName, current, total, speed));
}

void PluginBuildDownloader::downloadFinished()
{
    m_tempFile->seek(0);

    QDir libsDir(qtTubeApp->plugins().libraryLoadDirs().front());
    QDir pluginsDir(qtTubeApp->plugins().pluginLoadDirs().front());

    libsDir.mkpath(".");
    pluginsDir.mkpath(".");

    bool failure = false;
    std::optional<QFileInfo> pluginFile;

    if (m_fileName.endsWith(".zip"))
    {
        QZipReader zipReader(m_tempFile);
        const QList<QZipReader::FileInfo> files = zipReader.fileInfoList();

        for (auto it = files.begin(); it != files.end() && !failure; ++it)
        {
            const QString& path = it->filePath;
            if (it->isFile && QLibrary::isLibrary(path))
            {
                if (path.startsWith("libs/"))
                    failure = !writeFile(libsDir.filePath(path.section('/', -1)), zipReader.fileData(path));
                else if (!pluginFile && !path.contains('/'))
                    failure = !writeFile(pluginsDir.filePath(path), zipReader.fileData(path), &pluginFile);
            }
        }
    }
    else
    {
        failure = !writeFile(pluginsDir.filePath(m_fileName), m_tempFile->readAll(), &pluginFile);
    }

    if (!failure)
    {
        if (pluginFile)
        {
            try
            {
                PluginData* plugin = qtTubeApp->plugins().loadAndInitPlugin(pluginFile.value());
                if (!qtTubeApp->plugins().activePlugin() ||
                    QMessageBox::question(this, {}, "Make this plugin the active plugin?") == QMessageBox::Yes)
                {
                    plugin->active = true;
                    emit qtTubeApp->activePluginChanged(plugin);
                }
                emit success();
            }
            catch (const PluginLoadException& ex)
            {
                QMessageBox::critical(this, "Failed to Install Plugin", ex.message());
                QFile::remove(pluginFile->absoluteFilePath());
            }
        }
        else
        {
            QMessageBox::critical(this, "Failed to Install Plugin", "No plugin file found.");
        }
    }

    deleteLater();
}
