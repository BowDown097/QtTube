#include "youtubeplugin.h"
#include "innertube.h"
#include "localcache.h"
#include "qttube-plugin/replyutils.h"
#include "protobuf/protobufcompiler.h"
#include "utils/conversion.h"
#include "utils/replydata.h"
#include "utils/stringutils.h"
#include "youtubeplayer.h"
#include <QUrlQuery>

using namespace InnertubeEndpoints;

QtTubePlugin::PluginMetadata g_metadata = {
    .name = "YouTube",
    .description = "Built-in plugin for YouTube.",
    .image = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/YouTube_full-color_icon_%282017%29.svg/330px-YouTube_full-color_icon_%282017%29.svg.png",
    .author = "BowDown097",
    .url = "https://github.com/BowDown097/QtTube"
};
DECLARE_QTTUBE_PLUGIN(YouTubePlugin, g_metadata, YouTubePlayer, YouTubeSettings, YouTubeAuth)

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

QJsonValue g_liveChatSendEndpoint;
int g_liveChatSentMessages{};

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

QtTubePlugin::AccountReply* YouTubePlugin::getActiveAccount()
{
    QtTubePlugin::AccountReply* pluginReply = QtTubePlugin::AccountReply::create();
    InnertubeReply<AccountMenu>* accountMenuReply = InnerTube::instance()->get<AccountMenu>();
    InnertubeReply<UnseenCount>* unseenCountReply = InnerTube::instance()->get<UnseenCount>();

    auto data = std::make_shared<QtTubePlugin::InitialAccountData>();
    auto state = std::make_shared<QtTubePlugin::MultiCompletionState<2>>();

    QObject::connect(accountMenuReply, &InnertubeReply<AccountMenu>::exception, [=](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(accountMenuReply, &InnertubeReply<AccountMenu>::finished, [=](const AccountMenu& endpoint) {
        getAccountData(*data, endpoint.response);
        if (state->hit())
            emit pluginReply->finished(*data);
    });

    QObject::connect(unseenCountReply, &InnertubeReply<UnseenCount>::exception, [=](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(unseenCountReply, &InnertubeReply<UnseenCount>::finished, [=](const UnseenCount& endpoint) {
        data->notificationCount = endpoint.unseenCount;
        if (state->hit())
            emit pluginReply->finished(*data);
    });

    return pluginReply;
}

QtTubePlugin::ChannelReply* YouTubePlugin::getChannel(const QString& channelId, std::any tabData, std::any continuationData)
{
    QtTubePlugin::ChannelReply* pluginReply = QtTubePlugin::ChannelReply::create();

    QString continuationToken, params;
    if (const QString* cptr = std::any_cast<QString>(&continuationData))
        continuationToken = *cptr;
    if (const QString* pptr = std::any_cast<QString>(&tabData))
        params = *pptr;

    InnertubeReply<BrowseChannel>* tubeReply = InnerTube::instance()->get<BrowseChannel>(channelId, continuationToken, params);
    QObject::connect(tubeReply, &InnertubeReply<BrowseChannel>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<BrowseChannel>::finished, [pluginReply](const BrowseChannel& endpoint) {
        const auto& [continuationData, data] = getChannelData(endpoint.response);
        pluginReply->continuationData = continuationData;
        emit pluginReply->finished(data);
    });

    return pluginReply;
}

QtTubePlugin::BrowseReply* YouTubePlugin::getHistory(const QString& query, std::any continuationData)
{
    QtTubePlugin::BrowseReply* pluginReply = QtTubePlugin::BrowseReply::create();

    QString continuationToken;
    if (const QString* cptr = std::any_cast<QString>(&continuationData))
        continuationToken = *cptr;

    InnertubeReply<BrowseHistory>* tubeReply = InnerTube::instance()->get<BrowseHistory>(query, continuationToken);
    QObject::connect(tubeReply, &InnertubeReply<BrowseHistory>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<BrowseHistory>::finished, [pluginReply](const BrowseHistory& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getHistoryData(endpoint.response));
    });

    return pluginReply;
}

QtTubePlugin::BrowseReply* YouTubePlugin::getHome(std::any continuationData)
{
    QtTubePlugin::BrowseReply* pluginReply = QtTubePlugin::BrowseReply::create();

    QString continuationToken;
    if (const QString* cptr = std::any_cast<QString>(&continuationData))
        continuationToken = *cptr;

    if (InnerTube::instance()->hasAuthenticated())
    {
        InnertubeReply<BrowseHome>* tubeReply = InnerTube::instance()->get<BrowseHome>(continuationToken);
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

QtTubePlugin::LiveChatReply* YouTubePlugin::getLiveChat(std::any data)
{
    QtTubePlugin::LiveChatReply* pluginReply = QtTubePlugin::LiveChatReply::create();

    QString params;
    if (const QString* pptr = std::any_cast<QString>(&data))
        params = *pptr;

    InnertubeReply<GetLiveChat>* tubeReply = InnerTube::instance()->get<GetLiveChat>(params);
    QObject::connect(tubeReply, &InnertubeReply<GetLiveChat>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<GetLiveChat>::finished, [pluginReply](const GetLiveChat& endpoint) {
        emit pluginReply->finished(getLiveChatData(endpoint, g_liveChatSendEndpoint, g_liveChatSentMessages));
    });

    return pluginReply;
}

QtTubePlugin::LiveChatReplayReply* YouTubePlugin::getLiveChatReplay(std::any data, qint64 videoOffsetMs)
{
    QtTubePlugin::LiveChatReplayReply* pluginReply = QtTubePlugin::LiveChatReplayReply::create();

    QString params;
    if (const QString* pptr = std::any_cast<QString>(&data))
        params = *pptr;

    InnertubeReply<GetLiveChatReplay>* tubeReply = InnerTube::instance()->get<GetLiveChatReplay>(
        params, QString::number(videoOffsetMs));
    QObject::connect(tubeReply, &InnertubeReply<GetLiveChatReplay>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<GetLiveChatReplay>::finished, [pluginReply](const GetLiveChatReplay& endpoint) {
        emit pluginReply->finished(getLiveChatReplayData(endpoint));
    });

    return pluginReply;
}

QtTubePlugin::NotificationsReply* YouTubePlugin::getNotifications(std::any continuationData)
{
    QtTubePlugin::NotificationsReply* pluginReply = QtTubePlugin::NotificationsReply::create();

    QString continuationToken;
    if (const QString* cptr = std::any_cast<QString>(&continuationData))
        continuationToken = *cptr;

    InnertubeReply<GetNotificationMenu>* tubeReply = InnerTube::instance()->get<GetNotificationMenu>(
        "NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX", continuationToken);
    QObject::connect(tubeReply, &InnertubeReply<GetNotificationMenu>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<GetNotificationMenu>::finished, [pluginReply](const GetNotificationMenu& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getNotificationsData(endpoint.response));
    });

    return pluginReply;
}

QtTubePlugin::BrowseReply* YouTubePlugin::getSearch(
    const QString& query, const QList<std::pair<QString, int>>& activeFilters, std::any continuationData)
{
    QtTubePlugin::BrowseReply* pluginReply = QtTubePlugin::BrowseReply::create();

    QString continuationToken;
    if (const QString* cptr = std::any_cast<QString>(&continuationData))
        continuationToken = *cptr;

    InnertubeReply<Search>* tubeReply = InnerTube::instance()->get<Search>(
        query, continuationToken, compileSearchParams(activeFilters));
    QObject::connect(tubeReply, &InnertubeReply<Search>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<Search>::finished, [pluginReply](const Search& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getSearchData(endpoint.response));
    });

    return pluginReply;
}

QtTubePlugin::BrowseReply* YouTubePlugin::getSubFeed(std::any continuationData)
{
    QtTubePlugin::BrowseReply* pluginReply = QtTubePlugin::BrowseReply::create();

    QString continuationToken;
    if (const QString* cptr = std::any_cast<QString>(&continuationData))
        continuationToken = *cptr;

    InnertubeReply<BrowseSubscriptions>* tubeReply = InnerTube::instance()->get<BrowseSubscriptions>(continuationToken);
    QObject::connect(tubeReply, &InnertubeReply<BrowseSubscriptions>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<BrowseSubscriptions>::finished, [pluginReply](const BrowseSubscriptions& endpoint) {
        pluginReply->continuationData = endpoint.continuationToken;
        emit pluginReply->finished(getSubscriptionsData(endpoint.response));
    });

    return pluginReply;
}

QtTubePlugin::BrowseReply* YouTubePlugin::getTrending(std::any continuationData)
{
    QtTubePlugin::BrowseReply* pluginReply = QtTubePlugin::BrowseReply::create();

    InnertubeReply<BrowseTrending>* tubeReply = InnerTube::instance()->get<BrowseTrending>();
    QObject::connect(tubeReply, &InnertubeReply<BrowseTrending>::exception, [pluginReply](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(tubeReply, &InnertubeReply<BrowseTrending>::finished, [pluginReply](const BrowseTrending& endpoint) {
        emit pluginReply->finished(getTrendingData(endpoint.response));
    });

    return pluginReply;
}

QtTubePlugin::VideoReply* YouTubePlugin::getVideo(const QString& videoId, std::any continuationData)
{
    QtTubePlugin::VideoReply* pluginReply = QtTubePlugin::VideoReply::create();
    InnertubeReply<Next>* nextReply = InnerTube::instance()->get<Next>(videoId);
    InnertubeReply<Player>* playerReply = InnerTube::instance()->get<Player>(videoId);

    auto data = std::make_shared<QtTubePlugin::VideoData>();
    auto state = std::make_shared<QtTubePlugin::MultiCompletionState<2>>();

    QObject::connect(nextReply, &InnertubeReply<Next>::exception, [=](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(nextReply, &InnertubeReply<Next>::finished, [=](const Next& endpoint) {
        getNextData(*data, endpoint.response);
        if (state->hit())
            emit pluginReply->finished(*data);
    });

    QObject::connect(playerReply, &InnertubeReply<Player>::exception, [=](const InnertubeException& ex) {
        emit pluginReply->exception(convertException(ex));
    });
    QObject::connect(playerReply, &InnertubeReply<Player>::finished, [=](const Player& endpoint) {
        getPlayerData(*data, endpoint.response);
        if (state->hit())
            emit pluginReply->finished(*data);
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
}

QtTubePlugin::Reply<void>* YouTubePlugin::rate(const QString& videoId, bool like, bool removing, std::any data)
{
    QtTubePlugin::Reply<void>* pluginReply = QtTubePlugin::Reply<void>::create();

    QString params;
    if (const QString* pptr = std::any_cast<QString>(&data))
        params = *pptr;

    if (removing)
    {
        InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<RemoveLike>(videoId, params);
        QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTubePlugin::Reply<void>::finished);
    }
    else if (like)
    {
        InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Like>(videoId, params);
        QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTubePlugin::Reply<void>::finished);
    }
    else
    {
        InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Dislike>(videoId, params);
        QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTubePlugin::Reply<void>::finished);
    }

    return pluginReply;
}

QtTubePlugin::ResolveUrlReply* YouTubePlugin::resolveUrlOrID(const QString& in)
{
    QtTubePlugin::ResolveUrlReply* pluginReply = QtTubePlugin::ResolveUrlReply::create();

    static QRegularExpression channelRegex(R"((?:^|\/channel\/)(UC[a-zA-Z0-9_-]{22})(?=\b))");
    static QRegularExpression videoRegex(R"((?:^|v=|vi=|v\/|vi\/|watch\/|shorts\/|live\/|youtu\.be\/)([a-zA-Z0-9_-]{11})(?=\b))");

    auto noDomainMatch = [](const QString& host) {
        constexpr std::array YouTubeDomains = { "www.youtube.com", "youtube.com", "youtu.be" };
        return std::ranges::none_of(YouTubeDomains, [&host](const char* domain) {
            return host.compare(domain, Qt::CaseInsensitive) == 0;
        });
    };

    if (QRegularExpressionMatch channelMatch = channelRegex.match(in); channelMatch.lastCapturedIndex() >= 1)
    {
        QtTubePlugin::delayedEmit(pluginReply, QtTubePlugin::ResolveUrlData {
            .data = channelMatch.captured(1),
            .target = QtTubePlugin::ResolveUrlTarget::Channel
        });
    }
    else if (QRegularExpressionMatch videoMatch = videoRegex.match(in); videoMatch.lastCapturedIndex() >= 1)
    {
        bool continuePlayback{};
        qint64 progress{};

        if (QUrl url(in); url.isValid())
        {
            QUrlQuery urlQuery(url);
            if (urlQuery.hasQueryItem("t"))
                progress = urlQuery.queryItemValue("t").replace("s", "").toLongLong();
            if (urlQuery.queryItemValue("continuePlayback") == "1")
                continuePlayback = true;
        }

        QtTubePlugin::delayedEmit(pluginReply, QtTubePlugin::ResolveUrlData {
            .continuePlayback = continuePlayback,
            .data = videoMatch.captured(1),
            .target = QtTubePlugin::ResolveUrlTarget::Video,
            .videoProgress = progress
        });
    }
    else if (QUrl url(in); url.isValid() && (url.scheme() == "http" || url.scheme() == "https") && noDomainMatch(url.host()))
    {
        QtTubePlugin::delayedEmit(pluginReply, QtTubePlugin::ResolveUrlData {
            .data = in,
            .target = QtTubePlugin::ResolveUrlTarget::PlainUrl
        });
    }
    else
    {
        // this hits all kinds of stuff, but we're going to specifically filter for channels and videos.
        // doesn't have to be an exact URL either, so just giving a handle and stuff like that should still work
        InnertubeReply<ResolveUrl>* tubeReply = InnerTube::instance()->get<ResolveUrl>(in);
        QObject::connect(tubeReply, &InnertubeReply<ResolveUrl>::exception, [=](const InnertubeException& ex) {
            emit pluginReply->exception(convertException(ex));
        });
        QObject::connect(tubeReply, &InnertubeReply<ResolveUrl>::finished, [=](const ResolveUrl& endpoint) {
            QString webPageType = endpoint.endpoint["commandMetadata"]["webCommandMetadata"]["webPageType"].toString();
            if (webPageType == "WEB_PAGE_TYPE_CHANNEL")
            {
                emit pluginReply->finished(QtTubePlugin::ResolveUrlData {
                    .data = endpoint.endpoint["browseEndpoint"]["browseId"].toString(),
                    .target = QtTubePlugin::ResolveUrlTarget::Channel
                });
            }
            else if (webPageType == "WEB_PAGE_TYPE_WATCH")
            {
                emit pluginReply->finished(QtTubePlugin::ResolveUrlData {
                    .data = endpoint.endpoint["watchEndpoint"]["videoId"].toString(),
                    .target = QtTubePlugin::ResolveUrlTarget::Video,
                    .videoProgress = endpoint.endpoint["watchEndpoint"]["startTimeSeconds"].toInteger()
                });
            }
            else
            {
                // check for an edge case where a classic channel URL is returned instead of the UCID
                if (QString classicUrl = endpoint.endpoint["urlEndpoint"]["url"].toString(); !classicUrl.isEmpty())
                {
                    InnertubeReply<ResolveUrl>* tubeReply2 = InnerTube::instance()->get<ResolveUrl>(classicUrl);
                    QObject::connect(tubeReply2, &InnertubeReply<ResolveUrl>::exception, [=](const InnertubeException& ex2) {
                        emit pluginReply->exception(convertException(ex2));
                    });
                    QObject::connect(tubeReply2, &InnertubeReply<ResolveUrl>::finished, [=](const ResolveUrl& endpoint2) {
                        QString webPageType2 = endpoint2.endpoint["commandMetadata"]["webCommandMetadata"]["webPageType"].toString();
                        if (webPageType2 == "WEB_PAGE_TYPE_CHANNEL")
                        {
                            emit pluginReply->finished(QtTubePlugin::ResolveUrlData {
                                .data = endpoint2.endpoint["browseEndpoint"]["browseId"].toString(),
                                .target = QtTubePlugin::ResolveUrlTarget::Channel
                            });
                        }
                        else
                        {
                            emit pluginReply->finished(QtTubePlugin::ResolveUrlData());
                        }
                    });
                }
                else
                {
                    emit pluginReply->finished(QtTubePlugin::ResolveUrlData());
                }
            }
        });
    }

    return pluginReply;
}

QtTubePlugin::Reply<void>* YouTubePlugin::sendLiveChatMessage(const QString& text)
{
    QtTubePlugin::Reply<void>* pluginReply = QtTubePlugin::Reply<void>::create();

    const QString clientMessageId = g_liveChatSendEndpoint["clientIdPrefix"].toString() + QString::number(g_liveChatSentMessages++);
    const QString params = g_liveChatSendEndpoint["params"].toString();
    const QJsonArray textSegments = StringUtils::makeRichChatMessage(text);

    InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<SendMessage>(textSegments, clientMessageId, params);
    QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTubePlugin::Reply<void>::finished);

    return pluginReply;
}

QtTubePlugin::Reply<void>* YouTubePlugin::setNotificationPreference(std::any data)
{
    QtTubePlugin::Reply<void>* pluginReply = QtTubePlugin::Reply<void>::create();

    QString params;
    if (const QString* pptr = std::any_cast<QString>(&data))
        params = *pptr;

    InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<ModifyChannelPreference>(params);
    QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTubePlugin::Reply<void>::finished);

    return pluginReply;
}

QtTubePlugin::Reply<void>* YouTubePlugin::subscribe(std::any data)
{
    QtTubePlugin::Reply<void>* pluginReply = QtTubePlugin::Reply<void>::create();

    QString channelId, params;
    if (const QJsonValue* eptr = std::any_cast<QJsonValue>(&data))
    {
        channelId = (*eptr)["channelIds"].toArray().first().toString();
        params = (*eptr)["params"].toString();
    }

    InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Subscribe>(QStringList { channelId }, params);
    QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTubePlugin::Reply<void>::finished);

    return pluginReply;
}

QtTubePlugin::Reply<void>* YouTubePlugin::unsubscribe(std::any data)
{
    QtTubePlugin::Reply<void>* pluginReply = QtTubePlugin::Reply<void>::create();

    QString channelId, params;
    if (const QJsonValue* eptr = std::any_cast<QJsonValue>(&data))
    {
        channelId = (*eptr)["channelIds"].toArray().first().toString();
        params = (*eptr)["params"].toString();
    }

    InnertubeReply<void>* tubeReply = InnerTube::instance()->getPlain<Unsubscribe>(QStringList { channelId }, params);
    QObject::connect(tubeReply, &InnertubeReply<void>::finished, pluginReply, &QtTubePlugin::Reply<void>::finished);

    return pluginReply;
}
