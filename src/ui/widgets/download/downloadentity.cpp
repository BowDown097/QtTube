#include "downloadentity.h"
#include "src/ui/widgets/closebutton.h"
#include "src/utils/osutils.h"
#include "src/utils/stringutils.h"
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>

constexpr QLatin1String ytdlpTemplate(
    "%1 -P \"%2\" -q --progress --newline --progress-template "
    "\"download:[download] title:%(info.title)q downloaded_bytes:%(progress.downloaded_bytes)s "
    "total_bytes_estimate:%(progress.total_bytes_estimate)s total_bytes:%(progress.total_bytes)s "
    "progress.speed:%(progress.speed)s \" \"%3\""
);

// m_process has no parent! we're going to handle the deletion manualyly, see below.
DownloadEntity::DownloadEntity(const QDir& directory, QWidget* parent)
    : QProgressBar(parent), m_closeButton(new CloseButton(this)), m_directory(directory), m_process(new QProcess)
{
    m_closeButton->setFixedSize(16, 16);

    connect(m_closeButton, &CloseButton::clicked, this, std::bind(&DownloadEntity::finished, this, true));
    connect(m_process, &QProcess::finished, this, &DownloadEntity::handleFinished);
    connect(m_process, &QProcess::readyReadStandardError, this, &DownloadEntity::handleStandardError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &DownloadEntity::handleStandardOutput);
}

DownloadEntity::~DownloadEntity()
{
    // in some instances, these will still fire AFTER m_process has already been deleted.
    // so, we're going to manually disconnect them before that can happen.
    disconnect(m_process, &QProcess::finished, nullptr, nullptr);
    disconnect(m_process, &QProcess::readyReadStandardError, nullptr, nullptr);
    disconnect(m_process, &QProcess::readyReadStandardOutput, nullptr, nullptr);

    m_process->kill();
    connect(m_process, &QProcess::finished, m_process, &QProcess::deleteLater);
}

void DownloadEntity::bumpProgress(qint64 bytesReceived, qint64 bytesTotal, qint64 bytesPerSecond)
{
    setMaximum(bytesTotal);
    setValue(bytesReceived);

    const QString current = StringUtils::bytesString(bytesReceived);
    const QString speed = StringUtils::bytesString(bytesPerSecond);
    const QString total = StringUtils::bytesString(bytesTotal);

    setFormat(QStringLiteral("%1: %2 / %3 (%4/s)").arg(m_title, current, total, speed));
}

void DownloadEntity::cleanUp()
{
    if (m_title.isEmpty())
        return;

    const QStringList entries = m_directory.entryList(QStringList() << "*.part" << "*.ytdl");
    for (const QString& entry : entries)
        QFile::remove(entry);
}

QByteArray DownloadEntity::findEntry(const QByteArray& data, const QByteArray& entry)
{
    qsizetype index = data.indexOf(entry);
    if (index == -1)
        return "NA";

    const QByteArray entryData = data.mid(index + entry.size());
    index = entryData.front() == '\'' ? entryData.indexOf('\'', 1) : entryData.indexOf(' ');
    return index != -1 ? entryData.mid(0, index) : QByteArray("NA");
}

void DownloadEntity::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished(exitCode != 0 || exitStatus == QProcess::ExitStatus::CrashExit);
}

void DownloadEntity::handleStandardError()
{
    if (const QByteArray serr = m_process->readAllStandardError(); !serr.startsWith("WARNING"))
        QMessageBox::critical(nullptr, "Download Failed!", QStringLiteral("%1 encountered error: %2").arg(m_title, serr));
}

void DownloadEntity::handleStandardOutput()
{
    const QByteArray sout = m_process->readAllStandardOutput();
    if (!sout.startsWith("[download]"))
        return;

    if (!m_downloadStarted)
    {
        m_downloadStarted = true;
        emit requestSent();
    }

    // title could change from NA maybe, and findEntry is an inexpensive operation anyway, so worth retrying
    if (m_title.isEmpty() || m_title == "NA")
        m_title = findEntry(sout, "title:");

    const QByteArray downloadedBytes = findEntry(sout, "downloaded_bytes:");
    const QByteArray speed = findEntry(sout, "progress.speed:");

    if (const QByteArray totalBytes = findEntry(sout, "total_bytes:"); totalBytes != "NA")
        bumpProgress(downloadedBytes.toDouble(), totalBytes.toDouble(), speed.toDouble());
    else if (const QByteArray totalBytesEst = findEntry(sout, "total_bytes_estimate:"); totalBytesEst != "NA")
        bumpProgress(downloadedBytes.toDouble(), totalBytesEst.toDouble(), speed.toDouble());
}

void DownloadEntity::startDownload(const QUrl& url)
{
    const QString ytdlpPath = OSUtils::getFullPath(QFileInfo("yt-dlp"));
    if (ytdlpPath.isEmpty())
    {
        QMessageBox::warning(this, "yt-dlp not found!", "Could not find yt-dlp on your system. Make sure you have it in PATH or in this program's folder, then try again.");
        return;
    }

    m_process->startCommand(ytdlpTemplate.arg(ytdlpPath, m_directory.absolutePath(), url.toString()));
}
