#include "tubeutils.h"
#include "httprequest.h"
#include "innertube.h"
#include "qttubeapplication.h"

namespace TubeUtils
{
    QFuture<std::pair<QString, bool>> getSubCount(const QString& channelId, const QString& fallback)
    {
        QFutureInterface<std::pair<QString, bool>> futureInterface;
        futureInterface.reportStarted();

        if (!qtTubeApp->settings().fullSubs)
        {
            futureInterface.reportResult(std::make_pair(fallback, false));
            futureInterface.reportFinished();
            return futureInterface.future();
        }

        HttpReply* reply = HttpRequest().get("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelId);
        QObject::connect(reply, &HttpReply::finished, reply, [fallback, futureInterface](const HttpReply& reply) mutable {
            if (reply.isSuccessful())
            {
                static QRegularExpression estSubRegex("\"est_sub\":(\\d+)");
                if (QRegularExpressionMatch match = estSubRegex.match(reply.readAll()); match.hasCaptured(1))
                    futureInterface.reportResult(std::make_pair(QLocale::system().toString(match.captured(1).toInt()), true));
                else
                    futureInterface.reportResult(std::make_pair(fallback, false));
            }
            else
            {
                futureInterface.reportResult(std::make_pair(fallback, false));
            }

            futureInterface.reportFinished();
        });

        return futureInterface.future();
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
}
