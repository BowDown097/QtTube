#include "innertube.h"
#include "localcache.h"
#include "qttube-plugin/plugininterface.h"
#include "youtubeauth.h"
#include "youtubesettings.h"

class YouTubePlugin : public QtTube::PluginInterface
{
    void init() override;
};

static QtTube::PluginMetadata Metadata = {
    .name = "YouTube",
    .description = "Built-in plugin for YouTube.",
    .image = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/YouTube_full-color_icon_%282017%29.svg/330px-YouTube_full-color_icon_%282017%29.svg.png",
    .author = "BowDown097",
    .url = "https://github.com/BowDown097/QtTube"
};
DECLARE_QTTUBE_PLUGIN(YouTubePlugin, Metadata, YouTubeSettings, YouTubeAuth)

YouTubeAuth* authPtr = static_cast<YouTubeAuth*>(auth());
YouTubeSettings* settingsPtr = static_cast<YouTubeSettings*>(settings());

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

    if (const CredentialSet* activeLogin = dynamic_cast<const CredentialSet*>(authPtr->activeLogin()))
    {
        authPtr->populateAuthStore(*activeLogin);
        qDebug() << "from plugin:" << InnerTube::instance();
        if (InnerTube::instance()->hasAuthenticated())
            emit InnerTube::instance()->authStore()->authenticateSuccess();
    }
}
