#include "plugindownloaddialog.h"
#include "plugins/pluginbuilddownloader.h"
#include "qttubeapplication.h"
#include <QBoxLayout>
#include <QMessageBox>
#include <QProgressBar>

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

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

PluginDownloadDialog::PluginDownloadDialog(QString pluginName, ReleaseData data, QWidget* parent)
    : QWidget(parent),
      m_layout(new QVBoxLayout(this)),
      m_progressBar(new QProgressBar(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(width(), 100);
    setWindowTitle("Plugin Downloader");

    m_progressBar->setFixedHeight(50);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_progressBar);

    PluginBuildDownloader* downloader = new PluginBuildDownloader(std::move(pluginName), std::move(data), this);
    connect(downloader, &PluginBuildDownloader::failed, this, &PluginDownloadDialog::downloadFailed);
    connect(downloader, &PluginBuildDownloader::finished, this, &PluginDownloadDialog::downloadFinished);
    connect(downloader, &PluginBuildDownloader::progress, this, &PluginDownloadDialog::downloadProgress);
}

void PluginDownloadDialog::downloadFailed(const QString& error)
{
    QMessageBox::critical(this, "Failed to Install Plugin", error);
    deleteLater();
}

void PluginDownloadDialog::downloadFinished(PluginData* plugin)
{
    if (!qtTubeApp->plugins().activePlugin() ||
        QMessageBox::question(this, QString(), "Make this plugin the active plugin?") == QMessageBox::Yes)
    {
        plugin->active = true;
        emit qtTubeApp->activePluginChanged(plugin);
    }

    deleteLater();
    emit success();
}

void PluginDownloadDialog::downloadProgress(const QString& assetName, qint64 bytesReceived, qint64 bytesTotal)
{
    if (!m_timer.isValid())
        m_timer.start();

    m_progressBar->setMaximum(bytesTotal);
    m_progressBar->setValue(bytesReceived);

    QString current = bytesString(QStringLiteral("%1 %2"), bytesReceived);
    QString speed = bytesString(QStringLiteral("%1 %2/s"), bytesReceived * 1000.0 / m_timer.elapsed());
    QString total = bytesString(QStringLiteral("%1 %2"), bytesTotal);
    m_progressBar->setFormat(QStringLiteral("%1: %2 / %3 (%4)").arg(assetName, current, total, speed));
}
