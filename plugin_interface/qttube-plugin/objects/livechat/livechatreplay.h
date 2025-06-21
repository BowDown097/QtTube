#pragma once
#include "livechatbase.h"

namespace QtTube
{
    struct LiveChatReplayItem
    {
        LiveChatItem item;
        qint64 videoOffsetMs;
    };

    struct LiveChatReplay : LiveChatBase
    {
        QList<LiveChatReplayItem> items;
        std::any seekData;
    };
}
