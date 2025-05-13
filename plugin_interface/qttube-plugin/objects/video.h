#pragma once
#include <QList>
#include <QString>

namespace QtTube
{
    struct PluginVideoBadge
    {
        QString label;
        QString tooltip;
    };

    struct PluginVideo
    {
        QList<PluginVideoBadge> badges; // or tags
        QString lengthText;
        QString metadataText;
        int progressSecs{};
        QString sourceIconUrl;
        QString thumbnailUrl;
        QString title;
        QString uploaderAvatarUrl;
        QList<PluginVideoBadge> uploaderBadges;
        QString uploaderId;
        QString uploaderText;
        QString uploaderUrlPrefix;
        QString videoId;
        QString videoUrlPrefix;
    };
}
