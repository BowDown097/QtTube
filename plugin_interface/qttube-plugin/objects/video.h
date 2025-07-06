#pragma once
#include "badge.h"
#include <QTime>

namespace QtTubePlugin
{
    struct PluginMetadata;

    struct Video
    {
        bool isVerticalVideo{};
        QString lengthText;
        QString metadataText;
        int progressSecs{};
        PluginMetadata* sourceMetadata{};
        QList<Badge> tags; // or badges
        QString thumbnailUrl;
        QString title;
        QString uploaderAvatarUrl;
        QList<Badge> uploaderBadges;
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
