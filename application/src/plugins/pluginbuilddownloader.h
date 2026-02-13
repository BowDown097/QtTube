#pragma once
#include "plugins/pluginbrowser.h"

struct PluginData;
class QTemporaryFile;

class PluginBuildDownloader : public QObject
{
    Q_OBJECT
public:
    explicit PluginBuildDownloader(QString pluginName, ReleaseData data, QObject* parent = nullptr);
private:
    ReleaseData m_data;
    QString m_pluginName;
    QTemporaryFile* m_tempFile;

    void createUpdateIni(const QString& path);
private slots:
    void downloadFinished(const HttpReply& reply);
signals:
    void failed(const QString& message);
    void finished(PluginData* plugin);
    void progress(const QString& assetName, qint64 bytesReceived, qint64 bytesTotal);
};
