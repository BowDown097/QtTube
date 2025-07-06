#include "tubeutils.h"
#include "innertube.h"

namespace TubeUtils
{
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
