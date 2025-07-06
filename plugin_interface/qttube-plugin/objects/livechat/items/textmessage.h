#pragma once
#include <QString>

namespace QtTubePlugin
{
    struct TextMessage
    {
        QString authorAvatarUrl;
        QString authorName;
        QString authorNameColor;
        QString content;
        QString timestampText;
    };
}
