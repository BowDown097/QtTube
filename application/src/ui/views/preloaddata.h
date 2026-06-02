#pragma once
#include <QList>
#include <qttube-plugin/objects/badge.h>

namespace PreloadData
{
    struct WatchView
    {
        QString channelAvatarUrl;
        QList<QtTubePlugin::Badge> channelBadges;
        QString channelId;
        QString channelName;
        QString title;
    };
}
