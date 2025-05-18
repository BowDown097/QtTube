#pragma once
#include <QTime>

namespace QtTube
{
    struct PluginMetadata;

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
        PluginMetadata* sourceMetadata;
        QString thumbnailUrl;
        QString title;
        QString uploaderAvatarUrl;
        QList<PluginVideoBadge> uploaderBadges;
        QString uploaderId;
        QString uploaderText;
        QString uploaderUrlPrefix;
        QString videoId;
        QString videoUrlPrefix;

        QTime length() const
        {
            QTime out = QTime::fromString(lengthText, "h:mm:ss");
            if (!out.isValid())
                out = QTime::fromString(lengthText, "m:ss");
            return out;
        }
    };
}
