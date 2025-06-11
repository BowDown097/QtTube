#include "tubeutils.h"
#include "httprequest.h"
#include "innertube.h"
#include "protobuf/protobufutil.h"
#include <QRandomGenerator>
#include <QUrlQuery>

namespace TubeUtils
{
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp)
    {
        InnertubeContext* context = InnerTube::instance()->context();
        InnertubeAuthStore* authStore = InnerTube::instance()->authStore();

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
            { "cbr", context->client.browserName },
            { "cbrver", context->client.browserVersion },
            { "c", QString::number(static_cast<int>(context->client.clientType)) },
            { "cver", context->client.clientVersion },
            { "cplayer", "UNIPLAYER" },
            { "cos", context->client.osName },
            { "cosver", context->client.osVersion },
            { "cplatform", context->client.platform },
            { "hl", context->client.hl + "_" + context->client.gl },
            { "cr", context->client.gl },
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

        HttpRequest().withHeaders(InnertubeEndpoints::EndpointMethods::getNeededHeaders(context, authStore)).get(outPlaybackUrl);
    }

    // most logic courtesy of https://github.com/Rehike/Rehike
    InnertubeObjects::InnertubeString unattribute(const InnertubeObjects::DynamicText& attributedDescription)
    {
        const QString& content = attributedDescription.content;
        if (!attributedDescription.commandRuns.isArray())
            return InnertubeObjects::InnertubeString(content);

        const QJsonArray commandRuns = attributedDescription.commandRuns.toArray();
        InnertubeObjects::InnertubeString out;
        int start = 0;

        for (const QJsonValue& commandRun : commandRuns)
        {
            int length = commandRun["length"].toInt();
            int startIndex = commandRun["startIndex"].toInt();

            if (QString beforeText = content.mid(start, startIndex - start); !beforeText.isEmpty())
                out.runs.append(InnertubeObjects::InnertubeRun(beforeText));

            QString linkText = content.mid(startIndex, length);
            out.runs.append(InnertubeObjects::InnertubeRun(linkText, commandRun["onTap"]["innertubeCommand"]));

            start = startIndex + length;
        }

        if (QString lastText = content.mid(start); !lastText.isEmpty())
            out.runs.append(InnertubeObjects::InnertubeRun(lastText));

        return out;
    }
}
