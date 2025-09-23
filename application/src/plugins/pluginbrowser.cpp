#include "pluginbrowser.h"
#include "qttube-plugin/utils/httprequest.h"
#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>

using NodePtr = std::shared_ptr<HtmlParser::Node>;

namespace
{
    const QString downloadUrlFormat = QStringLiteral("https://raw.githubusercontent.com/%1/%2/metadata.json");
    const QString expandedAssetsFormat = QStringLiteral("https://github.com/%1/releases/expanded_assets/%2");
    const QString latestReleaseApiFormat = QStringLiteral("https://api.github.com/repos/%1/releases/latest");
    const QString latestReleaseWebFormat = QStringLiteral("https://github.com/%1/releases/latest");
    const QString nightlyLinkFormat = QStringLiteral("https://nightly.link/%1/workflows/build/%2");
    const QString searchReposFormat = QStringLiteral("https://api.github.com/search/repositories?q=topic:qttube-plugin&per_page=100&page=%1");

    QString getAssetName(const NodePtr& node)
    {
        std::string assetName;

        for (const NodePtr& child : node->Children)
        {
            if (child->Tag == "span")
            {
                assetName += child->GetTextContent();
                assetName += ' ';
            }
        }

        if (!assetName.empty() && assetName.back() == ' ')
            assetName.pop_back();

        return QString::fromStdString(assetName);
    }

    bool isReleaseForPlatform(const QUrl& url)
    {
    #if defined(Q_OS_WIN)
        constexpr QLatin1String platformName("Windows");
    #elif defined(Q_OS_MAC)
        constexpr QLatin1String platformName("macOS");
    #elif defined(Q_OS_LINUX)
        constexpr QLatin1String platformName("Linux");
    #else
        constexpr QLatin1String platformName("Unknown");
    #endif

        const QString fileName = url.fileName();
        return fileName.contains(platformName, Qt::CaseInsensitive) &&
               (QLibrary::isLibrary(fileName) || fileName.endsWith(".zip"));
    }
}

void PluginBrowser::getExpandedAssets(const PluginEntryMetadataPtr& metadata, const QString& tagName)
{
    HttpReply* reply = HttpRequest().get(expandedAssetsFormat.arg(metadata->repoFullName, tagName));
    connect(reply, &HttpReply::finished, this, [this, metadata, tagName](const HttpReply& reply) {
        getExpandedAssetsFinished(metadata, tagName, reply);
    });
}

void PluginBrowser::getExpandedAssetsFinished(
    const PluginEntryMetadataPtr& metadata, const QString& tagName, const HttpReply& reply)
{
    ReleaseData data = { .tagName = tagName };

    try
    {
        HtmlParser::Parser parser;
        parser.SetStrict(true);

        HtmlParser::DOM dom = parser.Parse(reply.readAll().toStdString());

        std::vector<NodePtr> items = dom.GetElementsByTagName("li");
        for (const NodePtr& item : items)
        {
            HtmlParser::Query query(item);
            if (NodePtr truncate = query.SelectFirst(".Truncate"))
            {
                if (QString href = QString::fromStdString(truncate->GetAttribute("href"));
                    isReleaseForPlatform(href))
                {
                    data.asset.downloadUrl = "https://github.com" + std::move(href);
                    data.asset.name = getAssetName(truncate);
                    if (NodePtr time = query.SelectFirst("relative-time"))
                        data.asset.updatedAt = QString::fromStdString(time->GetAttribute("datetime"));
                    else
                        data.asset.updatedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
                    break;
                }
            }
        }

        emit gotReleaseData(metadata, data);
    }
    catch (const std::runtime_error& ex)
    {
        emit error("Getting Release Data (fallback)", ex.what());
    }
}

void PluginBrowser::getRepositories()
{
    HttpReply* reply = HttpRequest().get(searchReposFormat.arg(m_reposPage));
    connect(reply, &HttpReply::finished, this, &PluginBrowser::getRepositoriesFinished);
}

void PluginBrowser::getRepositoriesFinished(const HttpReply& reply)
{
    QJsonObject obj = QJsonDocument::fromJson(reply.readAll()).object();

    if (auto [errorType, msg] = resolveError(reply, obj); errorType != ErrorType::NoError)
    {
        emit error("Getting Repositories", msg);
        return;
    }

    const QJsonArray itemsJson = obj["items"].toArray();
    PluginRepoCache& repoCache = cache().repos();
    repoCache.reserve(itemsJson.size());

    for (const QJsonValue& item : itemsJson)
    {
        repoCache.append(RepositoryItem {
            .defaultBranch = item["default_branch"].toString(),
            .fullName = item["full_name"].toString(),
            .ownerLogin = item["owner"]["login"].toString(),
            .url = item["url"].toString()
        });
    }

    emit gotRepositories(repoCache.data());

    int totalCount = obj["total_count"].toInt();
    int totalFetched = m_reposPage++ * repoCache.size();
    if (repoCache.isEmpty() && totalFetched < totalCount)
        getRepositories();
}

void PluginBrowser::releaseDataFallback(const PluginEntryMetadataPtr& metadata)
{
    HttpReply* reply = HttpRequest()
        .withAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy)
        .head(latestReleaseWebFormat.arg(metadata->repoFullName));
    connect(reply, &HttpReply::finished, this, [this, metadata](const HttpReply& reply) {
        releaseDataFallbackFinished(metadata, reply);
    });
}

void PluginBrowser::releaseDataFallbackFinished(const PluginEntryMetadataPtr& metadata, const HttpReply& reply)
{
    if (QVariant redirect = reply.attribute(QNetworkRequest::RedirectionTargetAttribute); redirect.isValid())
    {
        if (QString path = redirect.toUrl().path(); path.contains("/releases/tag/"))
        {
            getExpandedAssets(metadata, path.section('/', -1));
            return;
        }
    }

    // if we've gotten here, there's probably no release at all.
    // emit nullopt to prompt for nightly link fallback later
    emit gotReleaseData(metadata, std::nullopt);
}

std::pair<PluginBrowser::ErrorType, QString> PluginBrowser::resolveError(
    const HttpReply& reply, const QJsonObject& obj)
{
    int statusCode = reply.statusCode();
    if (statusCode >= 200 && statusCode < 300)
        return std::make_pair(ErrorType::NoError, QString());

    if (obj["message"].isString())
    {
        if (statusCode == 403 && reply.header("x-ratelimit-remaining").toInt() == 0)
        {
            QByteArray rateLimitReset = reply.header("x-ratelimit-reset");
            QString resetTime = QDateTime::fromSecsSinceEpoch(rateLimitReset.toLongLong())
                .toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
            return std::make_pair(ErrorType::RateLimit, "API rate limit exceeded. Resetting at " + resetTime);
        }

        return std::make_pair(ErrorType::Other, obj["message"].toString());
    }

    return std::make_pair(ErrorType::Other, "Failed with code " + QString::number(statusCode));
}

void PluginBrowser::tryGetMetadata(const RepositoryItemPtr& item, BasePluginEntry* entry)
{
    HttpReply* reply = HttpRequest().get(downloadUrlFormat.arg(item->fullName, item->defaultBranch));
    connect(reply, &HttpReply::finished, this, [this, entry, item](const HttpReply& reply) {
        tryGetMetadataFinished(entry, item, reply);
    });
}

void PluginBrowser::tryGetMetadataFinished(
    BasePluginEntry* entry, const RepositoryItemPtr& item, const HttpReply& reply)
{
    if (!reply.isSuccessful())
    {
        qWarning().noquote() << "Repository" << item->fullName << "has no metadata.";
        emit gotPluginMetadata(entry, nullptr);
        return;
    }

    QJsonObject obj = QJsonDocument::fromJson(reply.readAll()).object();

    auto [it, _] = cache().metadata().emplace(item->fullName, PluginEntryMetadata {
        .author = item->ownerLogin,
        .defaultBranch = item->defaultBranch,
        .description = obj["description"].toString(),
        .image = obj["image"].toString(),
        .name = obj["name"].toString(),
        .repoFullName = item->fullName,
        .url = obj["url"].toString(),
        .version = obj["version"].toString()
    });

    if (it->second->author.isEmpty() || it->second->name.isEmpty() || it->second->version.isEmpty())
    {
        qWarning().noquote() << "Metadata for" << item->fullName << "is missing one or more required fields.";
        emit gotPluginMetadata(entry, nullptr);
    }
    else
    {
        emit gotPluginMetadata(entry, it->second);
    }
}

void PluginBrowser::tryGetNightlyBuild(const PluginEntryMetadataPtr& metadata)
{
    HttpReply* reply = HttpRequest().get(
        nightlyLinkFormat.arg(metadata->repoFullName, metadata->defaultBranch));
    connect(reply, &HttpReply::finished, this, &PluginBrowser::tryGetNightlyBuildFinished);
}

void PluginBrowser::tryGetNightlyBuildFinished(const HttpReply& reply)
{
    if (int statusCode = reply.statusCode(); statusCode == 404)
    {
        emit error("Getting Nightly Build", "No nightly build is available.");
        return;
    }
    else if (statusCode < 200 || statusCode >= 300)
    {
        emit error("Getting Nightly Build", "Failed with code " + QString::number(reply.statusCode()));
        return;
    }

    try
    {
        HtmlParser::Parser parser;
        parser.SetStrict(true);

        HtmlParser::DOM dom = parser.Parse(reply.readAll().toStdString());
        HtmlParser::Query query(dom.Root());

        // child queries just don't work... gonna have to get creative
        std::vector<NodePtr> artifacts;
        {
            std::vector<NodePtr> downloadCells = query.Select("td");
            for (NodePtr& cell : downloadCells)
                for (NodePtr& child : cell->Children)
                    if (child->Tag == "a")
                        artifacts.emplace_back(std::move(child));
        }

        for (const NodePtr& artifact : artifacts)
        {
            QString href = QString::fromStdString(artifact->GetAttribute("href"));
            if (isReleaseForPlatform(href))
            {
                emit gotNightlyBuild(ReleaseData {
                    .asset = ReleaseData::Asset {
                        .downloadUrl = href,
                        .name = href.section('/', -1),
                        .updatedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
                    },
                    .tagName = "nightly"
                });
                return;
            }
        }

        emit error("Getting Nightly Build", "Could not find a suitable nightly build.");
    }
    catch (const std::runtime_error& ex)
    {
        emit error("Getting Nightly Build", ex.what());
    }
}

void PluginBrowser::tryGetReleaseData(const PluginEntryMetadataPtr& metadata)
{
    HttpReply* reply = HttpRequest().get(latestReleaseApiFormat.arg(metadata->repoFullName));
    connect(reply, &HttpReply::finished, this, [this, metadata](const HttpReply& reply) {
        tryGetReleaseDataFinished(metadata, reply);
    });
}

void PluginBrowser::tryGetReleaseDataFinished(const PluginEntryMetadataPtr& metadata, const HttpReply& reply)
{
    // if 404, then there's no release at all. emit nullopt to prompt for nightly link fallback later
    if (reply.statusCode() == 404)
    {
        emit gotReleaseData(metadata, std::nullopt);
        return;
    }

    QJsonObject obj = QJsonDocument::fromJson(reply.readAll()).object();

    if (auto [errorType, msg] = resolveError(reply, obj); errorType == ErrorType::RateLimit)
    {
        releaseDataFallback(metadata);
        return;
    }
    else if (errorType != ErrorType::NoError)
    {
        emit error("Getting Release Data", msg);
        return;
    }

    ReleaseData data = { .tagName = obj["tag_name"].toString() };

    const QJsonArray assets = obj["assets"].toArray();
    for (const QJsonValue& asset : assets)
    {
        if (QString downloadUrl = asset["browser_download_url"].toString(); isReleaseForPlatform(downloadUrl))
        {
            data.asset.downloadUrl = std::move(downloadUrl);
            data.asset.name = asset["name"].toString();
            data.asset.updatedAt = asset["updated_at"].toString();
            break;
        }
    }

    emit gotReleaseData(metadata, data);
}
