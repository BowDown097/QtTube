#pragma once
#include <QString>

namespace QtTube
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
