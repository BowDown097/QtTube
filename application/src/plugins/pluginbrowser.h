#pragma once
#include "pluginbrowsercache.h"
#include <QDateTime>

class HttpReply;

struct ReleaseData
{
    struct Asset
    {
        QString downloadUrl;
        QString name;
        QDateTime updatedAt;
    };

    std::optional<Asset> asset;
    QString defaultBranch;
    QString fullName;
    bool isNightly{};
};

class PluginBrowser : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    static PluginBrowserCache& cache() { static PluginBrowserCache _cache; return _cache; }

    void getMetadata(BasePluginEntry* entry, const RepositoryItemPtr& item);
    void getNightlyBuild(BasePluginEntry* entry, const QString& fullName, const QString& defaultBranch);
    void getReleaseData(BasePluginEntry* entry, const QString& fullName, const QString& defaultBranch);
    void getRepositories();
private:
    enum class ErrorType
    {
        NoError,
        RateLimit,
        Other
    };

    qint16 m_reposPage = 1;

    void getExpandedAssets(
        BasePluginEntry* entry, const QString& fullName,
        const QString& defaultBranch, const QString& tagName);
    void releaseDataFallback(BasePluginEntry* entry, const QString& fullName, const QString& defaultBranch);
    std::pair<ErrorType, QString> resolveError(const HttpReply& reply, const QJsonObject& obj);
private slots:
    void getExpandedAssetsFinished(
        BasePluginEntry* entry, const QString& fullName,
        const QString& defaultBranch, const HttpReply& reply);
    void getMetadataFinished(BasePluginEntry* entry, const RepositoryItemPtr& item, const HttpReply& reply);
    void getNightlyBuildFinished(
        BasePluginEntry* entry, const QString& fullName,
        const QString& defaultBranch, const HttpReply& reply);
    void getReleaseDataFinished(
        BasePluginEntry* entry, const QString& fullName,
        const QString& defaultBranch, const HttpReply& reply);
    void getRepositoriesFinished(const HttpReply& reply);
    void releaseDataFallbackFinished(
        BasePluginEntry* entry, const QString& fullName,
        const QString& defaultBranch, const HttpReply& reply);
signals:
    void error(const QString& context, const QString& message);
    void gotPluginMetadata(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata);
    void gotReleaseData(BasePluginEntry* entry, ReleaseData data);
    void gotRepositories(const QList<RepositoryItemPtr>& items);
};
