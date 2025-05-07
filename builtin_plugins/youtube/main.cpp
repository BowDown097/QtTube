#include "youtubesettings.h"
#include <QDebug>

class YouTubePlugin : public QtTube::PluginInterface
{
public:
    void init() override
    {
        qDebug() << "plugin initialized";
    }
};

DECLARE_QTTUBE_PLUGIN(
    YouTubePlugin,
    YouTubeSettings,
    .name = "YouTube",
    .description = "Built-in plugin for YouTube.",
    .image = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/YouTube_full-color_icon_%282017%29.svg/330px-YouTube_full-color_icon_%282017%29.svg.png",
    .author = "BowDown097",
    .url = "https://github.com/BowDown097/QtTube")
