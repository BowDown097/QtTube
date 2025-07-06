#pragma once
#include "livechatbase.h"

namespace QtTubePlugin
{
    struct LiveChat : LiveChatBase
    {
        QList<LiveChatItem> items;
        QString restrictedMessage;
    };
}
