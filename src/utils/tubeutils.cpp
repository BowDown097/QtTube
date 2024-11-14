#include "tubeutils.h"
#include "http.h"
#include "innertube.h"
#include <QRandomGenerator>
#include <QUrlQuery>

namespace TubeUtils
{
    QString generateCpn()
    {
        QString out;
        out.reserve(16);

        constexpr QLatin1String chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");
        for (int i = 0; i < out.capacity(); i++)
            out += chars[QRandomGenerator::global()->bounded(chars.size())];

        return out;
    }

    QString getUcidFromUrl(const QString& url)
    {
        QString ucid;

        try
        {
            auto reply = InnerTube::instance()->getBlocking<InnertubeEndpoints::ResolveUrl>(url);
            ucid = reply.endpoint["browseEndpoint"]["browseId"].toString();

            // check for an edge case where a classic channel URL is returned instead of the UCID
            if (ucid.isEmpty() && reply.endpoint["urlEndpoint"]["url"].isString())
            {
                QString classicUrl = reply.endpoint["urlEndpoint"]["url"].toString();
                auto reply2 = InnerTube::instance()->getBlocking<InnertubeEndpoints::ResolveUrl>(classicUrl);
                ucid = reply.endpoint["browseEndpoint"]["browseId"].toString();
            }
        }
        catch (const InnertubeException& ex)
        {
            qDebug() << ex.message();
        }

        return ucid;
    }

    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp)
    {
        InnertubeClient itc = InnerTube::instance()->context()->client;

        QUrlQuery playbackQuery(QUrl(playerResp.playbackTracking.videostatsPlaybackUrl));
        QUrl outPlaybackUrl("https://www.youtube.com/api/stats/playback");
        QUrlQuery outPlaybackQuery;

        QList<QPair<QString, QString>> map =
        {
            { "ns", "yt" },
            { "el", "detailpage" },
            { "cpn", generateCpn() },
            { "ver", "2" },
            { "fmt", "243" },
            { "fs", "0" },
            { "rt", QString::number(QRandomGenerator::global()->bounded(191) + 10) },
            { "euri", "" },
            { "lact", QString::number(QRandomGenerator::global()->bounded(7001) + 1000) },
            { "cl", playbackQuery.queryItemValue("cl") },
            { "mos", "0" },
            { "volume", "100" },
            { "cbr", itc.browserName },
            { "cbrver", itc.browserVersion },
            { "c", QString::number(static_cast<int>(itc.clientType)) },
            { "cver", itc.clientVersion },
            { "cplayer", "UNIPLAYER" },
            { "cos", itc.osName },
            { "cosver", itc.osVersion },
            { "cplatform", itc.platform },
            { "hl", itc.hl + "_" + itc.gl },
            { "cr", itc.gl },
            { "uga", playbackQuery.queryItemValue("uga") },
            { "len", playbackQuery.queryItemValue("len") },
            { "fexp", playbackQuery.queryItemValue("fexp") },
            { "rtn", "4" },
            { "afmt", "251" },
            { "muted", "0" },
            { "docid", playbackQuery.queryItemValue("docid") },
            { "ei", playbackQuery.queryItemValue("ei") },
            { "plid", playbackQuery.queryItemValue("plid") },
            { "sdetail", playbackQuery.queryItemValue("sdetail") },
            { "of", playbackQuery.queryItemValue("of") },
            { "vm", playbackQuery.queryItemValue("vm") }
        };

        outPlaybackQuery.setQueryItems(map);
        outPlaybackUrl.setQuery(outPlaybackQuery);

        Http http;
        http.setMaxRetries(0);
        setNeededHeaders(http, InnerTube::instance()->context(), InnerTube::instance()->authStore());
        http.get(outPlaybackUrl);
    }

    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore)
    {
        if (authStore->populated())
        {
            http.addRequestHeader("Authorization", authStore->generateSAPISIDHash().toUtf8());
            http.addRequestHeader("Cookie", authStore->toCookieString().toUtf8());
            http.addRequestHeader("X-Goog-AuthUser", "0");
        }

        http.addRequestHeader("Content-Type", "application/json");
        http.addRequestHeader("X-Goog-Visitor-Id", context->client.visitorData.toLatin1());
        http.addRequestHeader("X-YOUTUBE-CLIENT-NAME", QByteArray::number(static_cast<int>(context->client.clientType)));
        http.addRequestHeader("X-YOUTUBE-CLIENT-VERSION", context->client.clientVersion.toLatin1());
        http.addRequestHeader("X-ORIGIN", "https://www.youtube.com");
    }
}
