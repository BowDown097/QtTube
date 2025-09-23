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

    void getRepositories();
    void tryGetMetadata(const RepositoryItemPtr& item, BasePluginEntry* entry);
    void tryGetNightlyBuild(const PluginEntryMetadataPtr& metadata);
    void tryGetReleaseData(const PluginEntryMetadataPtr& metadata);
private:
    enum class ErrorType
    {
        NoError,
        RateLimit,
        Other
    };

    qint8 m_reposPage = 1;

    void getExpandedAssets(const PluginEntryMetadataPtr& metadata, const QString& tagName);
    void releaseDataFallback(const PluginEntryMetadataPtr& metadata);
    std::pair<ErrorType, QString> resolveError(const HttpReply& reply, const QJsonObject& obj);
private slots:
    void getExpandedAssetsFinished(
        const PluginEntryMetadataPtr& metadata, const QString& tagName, const HttpReply& reply);
    void getRepositoriesFinished(const HttpReply& reply);
    void releaseDataFallbackFinished(const PluginEntryMetadataPtr& metadata, const HttpReply& reply);
    void tryGetMetadataFinished(BasePluginEntry* entry, const RepositoryItemPtr& item, const HttpReply& reply);
    void tryGetNightlyBuildFinished(const HttpReply& reply);
    void tryGetReleaseDataFinished(const PluginEntryMetadataPtr& metadata, const HttpReply& reply);
signals:
    void error(const QString& context, const QString& message);
    void gotNightlyBuild(const ReleaseData& data);
    void gotPluginMetadata(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata);
    void gotReleaseData(const PluginEntryMetadataPtr& metadata, const std::optional<ReleaseData>& data);
    void gotRepositories(const QList<RepositoryItemPtr>& items);
};
