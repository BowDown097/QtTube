#pragma once
#include "plugins/pluginbrowser.hpp"
#include "plugins/pluginentry.hpp"
#include <QDir>
#include <QTemporaryFile>

class PluginBuildDownloader : public QObject
{
    Q_OBJECT
public:
    explicit PluginBuildDownloader(QDir pluginDir, ReleaseData data, QObject* parent = nullptr)
        : QObject(parent), m_data(std::move(data)), m_pluginDir(std::move(pluginDir)) {}
    void start();
private:
    ReleaseData m_data;
    QDir m_pluginDir;
    QTemporaryFile m_tempFile;

    void createUpdateIni(const QString& path);
private slots:
    void downloadFinished(const HttpReply& reply);
signals:
    void failed(const QString& message);
    void finished(PluginEntry* plugin);
    void progress(const QString& assetName, qint64 bytesReceived, qint64 bytesTotal);
};
