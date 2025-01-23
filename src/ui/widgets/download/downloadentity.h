#pragma once
#include <QDir>
#include <QProcess>
#include <QProgressBar>
#include <QUrl>

class CloseButton;

class DownloadEntity : public QProgressBar
{
    Q_OBJECT
public:
    explicit DownloadEntity(const QDir& directory, QWidget* parent = nullptr);
    ~DownloadEntity();

    void cleanUp();
    void startDownload(const QUrl& url);

    const QString& title() const { return m_title; }
    const QUrl& url() const { return m_url; }
private:
    CloseButton* m_closeButton;
    QDir m_directory;
    bool m_downloadStarted{};
    QProcess* m_process;
    QString m_title;
    QUrl m_url;

    void bumpProgress(qint64 bytesReceived, qint64 bytesTotal, qint64 bytesPerSecond);
    QByteArray findEntry(const QByteArray& data, const QByteArray& entry);
private slots:
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleStandardError();
    void handleStandardOutput();
signals:
    void finished(bool cancelled);
    void requestSent();
};
