#include "youtubeplugin.h"
#include "innertube.h"
#include "localcache.h"
#include "protobuf/protobufcompiler.h"
#include "utils/conversion.h"
#include "utils/replydata.h"

using namespace InnertubeEndpoints;

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

const QMap<int, QString> g_searchFeatureMap = {
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
            filterParams.insert(g_searchFeatureMap[index], true);
        else if (category == "Sort by")
            params.insert("sort", index);
    }

    if (!filterParams.isEmpty())
        params.insert("filter", filterParams);

    return !params.isEmpty() ? ProtobufCompiler::compileEncoded(params, g_searchMsgFields) : QByteArray();
}

QtTube::BrowseReply* YouTubePlugin::getHistory(const QString& query, std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeReply<BrowseHistory>* tubeReply = InnerTube::instance()->get<BrowseHistory>(
        query, std::any_cast<QString>(continuationData));
    QObject::connect(tubeReply, &InnertubeReply<BrowseHistory>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<BrowseHistory>::finished, [pluginReply](const BrowseHistory& endpoint) {
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
        InnertubeReply<BrowseHome>* tubeReply = InnerTube::instance()->get<BrowseHome>(
            std::any_cast<QString>(continuationData));
        QObject::connect(tubeReply, &InnertubeReply<BrowseHome>::exception, [pluginReply](const InnertubeException& ex) {
            emit pluginReply->exception(convertException(ex));
        });
        QObject::connect(tubeReply, &InnertubeReply<BrowseHome>::finished, [pluginReply](const BrowseHome& endpoint) {
            pluginReply->continuationData = endpoint.continuationToken;
            emit pluginReply->finished(getHomeData(endpoint.response));
        });
    }
    else
    {
        InnertubeReply<BrowseHome>* tubeReply = InnerTube::instance()->getRaw<BrowseHome>({
            { "context", QJsonObject {
                { "client", QJsonObject {
                    { "clientName", static_cast<int>(InnertubeClient::ClientType::IOS_UNPLUGGED) },
                    { "clientVersion", InnertubeClient::getLatestVersion(InnertubeClient::ClientType::IOS_UNPLUGGED) }
                }}
            }}
        });

        QObject::connect(tubeReply, &InnertubeReply<BrowseHome>::exception, [pluginReply](const InnertubeException& ex) {
            emit pluginReply->exception(convertException(ex));
        });
        QObject::connect(tubeReply, &InnertubeReply<BrowseHome>::finishedRaw, [pluginReply](const QJsonValue& data) {
            if (const nonstd::expected<BrowseHome, InnertubeException> endpoint = InnerTube::tryCreate<BrowseHome>(data))
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

    InnertubeReply<GetNotificationMenu>* tubeReply = InnerTube::instance()->get<GetNotificationMenu>(
        "NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX", std::any_cast<QString>(continuationData));
    QObject::connect(tubeReply, &InnertubeReply<GetNotificationMenu>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<GetNotificationMenu>::finished, [pluginReply](const GetNotificationMenu& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getNotificationsData(endpoint.response));
    });

    return pluginReply;
}

QtTube::BrowseReply* YouTubePlugin::getSearch(
    const QString& query, const QList<std::pair<QString, int>>& activeFilters, std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeReply<Search>* tubeReply = InnerTube::instance()->get<Search>(
        query, std::any_cast<QString>(continuationData), compileSearchParams(activeFilters));
    QObject::connect(tubeReply, &InnertubeReply<Search>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<Search>::finished, [pluginReply](const Search& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getSearchData(endpoint.response));
    });

    return pluginReply;
}

QtTube::BrowseReply* YouTubePlugin::getSubFeed(std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeReply<BrowseSubscriptions>* tubeReply = InnerTube::instance()->get<BrowseSubscriptions>(
        std::any_cast<QString>(continuationData));
    QObject::connect(tubeReply, &InnertubeReply<BrowseSubscriptions>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<BrowseSubscriptions>::finished, [pluginReply](const BrowseSubscriptions& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getSubscriptionsData(endpoint.response));
    });

    return pluginReply;
}

QtTube::BrowseReply* YouTubePlugin::getTrending(std::any continuationData)
{
    QtTube::BrowseReply* pluginReply = QtTube::BrowseReply::create();

    InnertubeReply<BrowseTrending>* tubeReply = InnerTube::instance()->get<BrowseTrending>();
    QObject::connect(tubeReply, &InnertubeReply<BrowseTrending>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<BrowseTrending>::finished, [pluginReply](const BrowseTrending& endpoint) {
        emit pluginReply->finished(getTrendingData(endpoint.response));
    });

    return pluginReply;
}

QtTube::VideoReply* YouTubePlugin::getVideo(const QString& videoId, std::any continuationData)
{
    std::shared_ptr<QtTube::VideoData> videoData = std::make_shared<QtTube::VideoData>();
    QtTube::VideoReply* pluginReply = QtTube::VideoReply::create();

    InnertubeReply<Next>* nextReply = InnerTube::instance()->get<Next>(videoId);
    InnertubeReply<Player>* playerReply = InnerTube::instance()->get<Player>(videoId);

    struct CompletionState { bool nextFinished{}; bool playerFinished{}; QMutex mutex; };
    std::shared_ptr<CompletionState> state = std::make_shared<CompletionState>();

    auto tryEmitFinished = [=] {
        QMutexLocker locker(&state->mutex);
        if (state->nextFinished && state->playerFinished)
            emit pluginReply->finished(*videoData);
    };

    QObject::connect(nextReply, &InnertubeReply<Next>::exception, [=](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(nextReply, &InnertubeReply<Next>::finished, [=](const Next& endpoint) {
        getNextData(*videoData, endpoint.response);
        state->nextFinished = true;
        tryEmitFinished();
    });

    QObject::connect(playerReply, &InnertubeReply<Player>::exception, [=](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(playerReply, &InnertubeReply<Player>::finished, [=](const Player& endpoint) {
        getPlayerData(*videoData, endpoint.response);
        state->playerFinished = true;
        tryEmitFinished();
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

QtTube::PluginReply<void>* YouTubePlugin::rate(const QString& videoId, bool like, bool removing, std::any data)
{
    QtTube::PluginReply<void>* pluginReply = QtTube::PluginReply<void>::create();

    if (removing)
    {
        InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<RemoveLike>(videoId, std::any_cast<QString>(data));
        QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTube::PluginReply<void>::finished);
    }
    else if (like)
    {
        InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Like>(videoId, std::any_cast<QString>(data));
        QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTube::PluginReply<void>::finished);
    }
    else
    {
        InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Dislike>(videoId, std::any_cast<QString>(data));
        QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTube::PluginReply<void>::finished);
    }

    return pluginReply;
}

QtTube::PluginReply<void>* YouTubePlugin::setNotificationPreference(std::any data)
{
    QtTube::PluginReply<void>* pluginReply = QtTube::PluginReply<void>::create();

    InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<ModifyChannelPreference>(std::any_cast<QString>(data));
    QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTube::PluginReply<void>::finished);

    return pluginReply;
}

QtTube::PluginReply<void>* YouTubePlugin::subscribe(std::any data)
{
    QtTube::PluginReply<void>* pluginReply = QtTube::PluginReply<void>::create();

    const QJsonValue endpoint = std::any_cast<QJsonValue>(data);
    const QString channelId = endpoint["channelIds"].toArray().first().toString();
    const QString params = endpoint["params"].toString();

    InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Subscribe>(QStringList { channelId }, params);
    QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTube::PluginReply<void>::finished);

    return pluginReply;
}

QtTube::PluginReply<void>* YouTubePlugin::unsubscribe(std::any data)
{
    QtTube::PluginReply<void>* pluginReply = QtTube::PluginReply<void>::create();

    const QJsonValue endpoint = std::any_cast<QJsonValue>(data);
    const QString channelId = endpoint["channelIds"].toArray().first().toString();
    const QString params = endpoint["params"].toString();

    InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Unsubscribe>(QStringList { channelId }, params);
    QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTube::PluginReply<void>::finished);

    return pluginReply;
}
