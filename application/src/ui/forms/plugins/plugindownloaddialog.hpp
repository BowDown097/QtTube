#pragma once
#include "plugins/pluginentry.hpp"
#include "plugins/releasedata.hpp"
#include <QElapsedTimer>
#include <QWidget>

class QProgressBar;
class QVBoxLayout;

class PluginDownloadDialog : public QWidget
{
    Q_OBJECT
public:
    explicit PluginDownloadDialog(QString pluginName, ReleaseData data, QWidget* parent = nullptr);
private:
    QVBoxLayout* m_layout;
    QProgressBar* m_progressBar;
    QElapsedTimer m_timer;
private slots:
    void downloadFailed(const QString& error);
    void downloadFinished(PluginEntry* plugin);
    void downloadProgress(const QString& assetName, qint64 bytesReceived, qint64 bytesTotal);
signals:
    void success();
};
