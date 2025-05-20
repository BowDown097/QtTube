#include "youtubeplugin.h"
#include "innertube.h"
#include "localcache.h"
#include "protobuf/protobufcompiler.h"
#include "utils/conversion.h"
#include "utils/replydata.h"

QtTube::PluginMetadata g_metadata = {
    .name = "YouTube",
    .description = "Built-in plugin for YouTube.",
    .image = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/YouTube_full-color_icon_%282017%29.svg/330px-YouTube_full-color_icon_%282017%29.svg.png",
    .author = "BowDown097",
    .url = "https://github.com/BowDown097/QtTube"
};
DECLARE_QTTUBE_PLUGIN(YouTubePlugin, g_metadata, YouTubeSettings, YouTubeAuth)

YouTubeAuth* g_auth = static_cast<YouTubeAuth*>(auth());
YouTubeSettings* g_settings = static_cast<YouTubeSettings*>(settings());

const QMap<int, QString> g_featureMap = {
    { 0, "isLive" },
    { 1, "is4K" },
    { 2, "isHD" },
    { 3, "hasSubtitles" },
    { 4, "isCreativeCommons" },
    { 5, "is360Degree" },
    { 6, "isVR180" },
    { 7, "is3D" },
    { 8, "isHDR" },
    { 9, "hasLocation" },
    { 10, "isPurchased" },
};

const QVariantMap g_searchMsgFields = {
    { "sort", QVariantList{1, 0} },
    {
        "filter", QVariantList{2, 2, QVariantMap{
            { "uploadDate", QVariantList{1, 0} },
            { "type", QVariantList{2, 0} },
            { "duration", QVariantList{3, 0} },
            { "isHD", QVariantList{4, 0} },
            { "hasSubtitles", QVariantList{5, 0} },
            { "isCreativeCommons", QVariantList{6, 0} },
            { "is3D", QVariantList{7, 0} },
            { "isLive", QVariantList{8, 0} },
            { "isPurchased", QVariantList{9, 0} },
            { "is4K", QVariantList{14, 0} },
            { "is360Degree", QVariantList{15, 0} },
            { "hasLocation", QVariantList{23, 0} },
            { "isHDR", QVariantList{25, 0} },
            { "isVR180", QVariantList{26, 0} }
        }}
    }
};

using InnertubeHistoryReply = InnertubeReply<InnertubeEndpoints::BrowseHistory>;
using InnertubeNotificationsReply = InnertubeReply<InnertubeEndpoints::GetNotificationMenu>;
using InnertubeHomeReply = InnertubeReply<InnertubeEndpoints::BrowseHome>;
using InnertubeSearchReply = InnertubeReply<InnertubeEndpoints::Search>;
using InnertubeSubsReply = InnertubeReply<InnertubeEndpoints::BrowseSubscriptions>;
using InnertubeTrendingReply = InnertubeReply<InnertubeEndpoints::BrowseTrending>;

QByteArray YouTubePlugin::compileSearchParams(const QList<std::pair<QString, int>>& activeFilters)
{
    QVariantMap filterParams, params;
    for (const auto& [category, index] : activeFilters)
    {
        if (category == "Upload date")
            filterParams.insert("uploadDate", index + 1);
        else if (category == "Type")
            filterParams.insert("type", index + 1);
        else if (category == "Duration")
            filterParams.insert("duration", index + 1);
        else if (category == "Features")
            filterParams.insert(g_featureMap[index], true);
        else if (category == "Sort by")
            params.insert("sort", index);
    }

    if (!filterParams.isEmpty())
        params.insert("filter", filterParams);

    QByteArray compiledParams;
    if (!params.isEmpty())
        compiledParams = ProtobufCompiler::compileEncoded(params, g_searchMsgFields);

    return compiledParams;
}

QString YouTubePlugin::getContinuationToken(std::any continuationData)
{
    QString continuationToken;
    if (QString* ctoken = std::any_cast<QString>(&continuationData))
        continuationToken = *ctoken;
    return continuationToken;
}

QtTube::BrowseReply* YouTubePlugin::getHistory(const QString& query, std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeHistoryReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::BrowseHistory>(query, getContinuationToken(continuationData));
    QObject::connect(tubeReply, &InnertubeHistoryReply::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeHistoryReply::finished, [pluginReply](const InnertubeEndpoints::BrowseHistory& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getHistoryData(endpoint.response));
    });

    return pluginReply;
}

QtTube::BrowseReply* YouTubePlugin::getHome(std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    if (InnerTube::instance()->hasAuthenticated())
    {
        InnertubeHomeReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::BrowseHome>(getContinuationToken(continuationData));
        QObject::connect(tubeReply, &InnertubeHomeReply::exception, [pluginReply](const InnertubeException& ex) {
            emit pluginReply->exception(convertException(ex));
        });
        QObject::connect(tubeReply, &InnertubeHomeReply::finished, [pluginReply](const InnertubeEndpoints::BrowseHome& endpoint) {
            pluginReply->continuationData = endpoint.continuationToken;
            emit pluginReply->finished(getHomeData(endpoint.response));
        });
    }
    else
    {
        InnertubeHomeReply* tubeReply = InnerTube::instance()->getRaw<InnertubeEndpoints::BrowseHome>({
            { "context", QJsonObject {
                { "client", QJsonObject {
                    { "clientName", static_cast<int>(InnertubeClient::ClientType::IOS_UNPLUGGED) },
                    { "clientVersion", InnertubeClient::getLatestVersion(InnertubeClient::ClientType::IOS_UNPLUGGED) }
                }}
            }}
        });

        QObject::connect(tubeReply, &InnertubeHomeReply::exception, [pluginReply](const InnertubeException& ex) {
            emit pluginReply->exception(convertException(ex));
        });
        QObject::connect(tubeReply, &InnertubeHomeReply::finishedRaw, [pluginReply](const QJsonValue& data) {
            if (const auto endpoint = InnerTube::tryCreate<InnertubeEndpoints::BrowseHome>(data))
            {
                pluginReply->continuationData = endpoint->continuationToken;
                emit pluginReply->finished(getHomeData(endpoint->response));
            }
            else
            {
                emit pluginReply->exception(convertException(endpoint.error()));
            }
        });
    }

    return pluginReply;
}

QtTube::NotificationsReply* YouTubePlugin::getNotifications(std::any continuationData)
{
    QtTube::NotificationsReply* pluginReply = QtTube::NotificationsReply::create();

    InnertubeNotificationsReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::GetNotificationMenu>(
        "NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX", getContinuationToken(continuationData));
    QObject::connect(tubeReply, &InnertubeNotificationsReply::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeNotificationsReply::finished, [pluginReply](const InnertubeEndpoints::GetNotificationMenu& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getNotificationsData(endpoint.response));
    });

    return pluginReply;
}

QtTube::BrowseReply* YouTubePlugin::getSearch(
    const QString& query, const QList<std::pair<QString, int>>& activeFilters, std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeSearchReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::Search>(
        query, getContinuationToken(continuationData), compileSearchParams(activeFilters));
    QObject::connect(tubeReply, &InnertubeSearchReply::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeSearchReply::finished, [pluginReply](const InnertubeEndpoints::Search& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getSearchData(endpoint.response));
    });

    return pluginReply;
}

QtTube::BrowseReply* YouTubePlugin::getSubFeed(std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeSubsReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::BrowseSubscriptions>(getContinuationToken(continuationData));
    QObject::connect(tubeReply, &InnertubeSubsReply::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeSubsReply::finished, [pluginReply](const InnertubeEndpoints::BrowseSubscriptions& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getSubscriptionsData(endpoint.response));
    });

    return pluginReply;
}

QtTube::BrowseReply* YouTubePlugin::getTrending(std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeTrendingReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::BrowseTrending>();
    QObject::connect(tubeReply, &InnertubeTrendingReply::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeTrendingReply::finished, [pluginReply](const InnertubeEndpoints::BrowseTrending& endpoint) {
        emit pluginReply->finished(getTrendingData(endpoint.response));
    });

    return pluginReply;
}

void YouTubePlugin::init()
{
    LocalCache* cache = LocalCache::instance("yt-client");
    cache->setMaxSeconds(86400);

    if (const QByteArray cver = cache->value("cver"); !cver.isNull())
    {
        InnerTube::instance()->createClient(InnertubeClient::ClientType::WEB, cver);
    }
    else
    {
        cache->clear();
        InnerTube::instance()->createClient(InnertubeClient::ClientType::WEB, "2.20250421.01.00", true);
        cache->insert("cver", InnerTube::instance()->context()->client.clientVersion.toLatin1());
    }

    if (const CredentialSet* activeLogin = static_cast<const CredentialSet*>(g_auth->activeLogin()))
    {
        g_auth->populateAuthStore(*activeLogin);
        if (InnerTube::instance()->hasAuthenticated())
            emit InnerTube::instance()->authStore()->authenticateSuccess();
    }
}
