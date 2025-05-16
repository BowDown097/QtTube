#include "youtubeplugin.h"
#include "innertube.h"
#include "localcache.h"
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

using InnertubeHomeReply = InnertubeReply<InnertubeEndpoints::BrowseHome>;
using InnertubeTrendingReply = InnertubeReply<InnertubeEndpoints::BrowseTrending>;

QtTube::HomeReply* YouTubePlugin::getHome(std::any data)
{
    QtTube::HomeReply* pluginReply = QtTube::HomeReply::create();

    QString continuationToken;
    if (QString* ctoken = std::any_cast<QString>(&data))
        continuationToken = *ctoken;

    if (InnerTube::instance()->hasAuthenticated())
    {
        InnertubeHomeReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::BrowseHome>(continuationToken);
        QObject::connect(tubeReply, &InnertubeHomeReply::exception, [pluginReply](const InnertubeException& ex) {
            emit pluginReply->exception(convertException(ex));
        });
        QObject::connect(tubeReply, &InnertubeHomeReply::finished, [pluginReply](const InnertubeEndpoints::BrowseHome& endpoint) {
            pluginReply->continuationData = endpoint.continuationToken;
            emit pluginReply->finished(getHomeData(endpoint));
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
                emit pluginReply->finished(getHomeData(endpoint.value()));
            }
            else
            {
                emit pluginReply->exception(convertException(endpoint.error()));
            }
        });
    }

    return pluginReply;
}

QtTube::TrendingReply* YouTubePlugin::getTrending(std::any data)
{
    QtTube::TrendingReply* pluginReply = QtTube::TrendingReply::create();

    InnertubeTrendingReply* tubeReply = InnerTube::instance()->get<InnertubeEndpoints::BrowseTrending>();
    QObject::connect(tubeReply, &InnertubeTrendingReply::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeTrendingReply::finished, [pluginReply](const InnertubeEndpoints::BrowseTrending& endpoint) {
        emit pluginReply->finished(getTrendingData(endpoint));
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
