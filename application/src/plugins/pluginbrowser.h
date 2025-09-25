#pragma once
#include "pluginbrowsercache.h"

class HttpReply;

struct ReleaseData
{
    struct Asset
    {
        QString downloadUrl;
        QString name;
        QString updatedAt;
    };

    Asset asset;
    QString tagName;
};

class PluginBrowser : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    static PluginBrowserCache& cache() { static PluginBrowserCache _cache; return _cache; }

    void getMetadata(BasePluginEntry* entry, const RepositoryItemPtr& item);
    void getNightlyBuild(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata);
    void getReleaseData(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata);
    void getRepositories();
private:
    enum class ErrorType
    {
        NoError,
        RateLimit,
        Other
    };

    qint8 m_reposPage = 1;

    void getExpandedAssets(
        BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata, const QString& tagName);
    void releaseDataFallback(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata);
    std::pair<ErrorType, QString> resolveError(const HttpReply& reply, const QJsonObject& obj);
private slots:
    void getExpandedAssetsFinished(
        BasePluginEntry* entry,
        const PluginEntryMetadataPtr& metadata,
        const QString& tagName,
        const HttpReply& reply);
    void getMetadataFinished(BasePluginEntry* entry, const RepositoryItemPtr& item, const HttpReply& reply);
    void getNightlyBuildFinished(BasePluginEntry* entry, const HttpReply& reply);
    void getReleaseDataFinished(
        BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata, const HttpReply& reply);
    void getRepositoriesFinished(const HttpReply& reply);
    void releaseDataFallbackFinished(
        BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata, const HttpReply& reply);
signals:
    void error(const QString& context, const QString& message);
    void gotNightlyBuild(BasePluginEntry* entry, const ReleaseData& data);
    void gotPluginMetadata(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata);
    void gotReleaseData(
        BasePluginEntry* entry,
        const PluginEntryMetadataPtr& metadata,
        const std::optional<ReleaseData>& data);
    void gotRepositories(const QList<RepositoryItemPtr>& items);
};
