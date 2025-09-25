#pragma once
#include <QElapsedTimer>
#include <QWidget>

class QProgressBar;
class QTemporaryFile;
class QVBoxLayout;
struct ReleaseData;

class PluginBuildDownloader : public QWidget
{
    Q_OBJECT
public:
    explicit PluginBuildDownloader(const ReleaseData& data, QWidget* parent = nullptr);
private:
    QString m_assetName;
    QString m_fileName;
    QVBoxLayout* m_layout;
    QProgressBar* m_progressBar;
    QTemporaryFile* m_tempFile;
    QElapsedTimer m_timer;
private slots:
    void downloadFinished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
signals:
    void success();
};
