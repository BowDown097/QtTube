#include "tubeutils.h"
#include "httprequest.h"
#include "innertube.h"
#include "protobuf/protobufutil.h"
#include <QRandomGenerator>
#include <QUrlQuery>

namespace TubeUtils
{
    QMap<QByteArray, QByteArray> getNeededHeaders()
    {
        InnertubeAuthStore* authStore = InnerTube::instance()->authStore();
        InnertubeContext* context = InnerTube::instance()->context();
        QMap<QByteArray, QByteArray> headers;

        if (authStore->populated())
        {
            headers.insert("Authorization", authStore->generateSAPISIDHash().toUtf8());
            headers.insert("Cookie", authStore->toCookieString().toUtf8());
            headers.insert("X-Goog-AuthUser", "0");
        }

        headers.insert("Content-Type", "application/json");
        headers.insert("X-Goog-Visitor-Id", context->client.visitorData.toLatin1());
        headers.insert("X-YOUTUBE-CLIENT-NAME", QByteArray::number(static_cast<int>(context->client.clientType)));
        headers.insert("X-YOUTUBE-CLIENT-VERSION", context->client.clientVersion.toLatin1());
        headers.insert("X-ORIGIN", "https://www.youtube.com");

        return headers;
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
            { "cpn", ProtobufUtil::randomString(16) },
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

        HttpRequest().withHeaders(getNeededHeaders()).get(outPlaybackUrl);
    }
}
