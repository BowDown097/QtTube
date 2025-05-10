#include "youtubeauth.h"
#include "youtubesettings.h"

class YouTubePlugin : public QtTube::PluginInterface
{
    void init() override;
};

DECLARE_QTTUBE_PLUGIN(
    YouTubePlugin,
    YouTubeSettings,
    YouTubeAuth,
    .name = "YouTube",
    .description = "Built-in plugin for YouTube.",
    .image = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/YouTube_full-color_icon_%282017%29.svg/330px-YouTube_full-color_icon_%282017%29.svg.png",
    .author = "BowDown097",
    .url = "https://github.com/BowDown097/QtTube")

YouTubeAuth* authPtr = static_cast<YouTubeAuth*>(authentication());
YouTubeSettings* settingsPtr = static_cast<YouTubeSettings*>(settings());

void YouTubePlugin::init()
{
    qDebug() << "plugin initialized";
}
