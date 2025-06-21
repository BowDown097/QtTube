#pragma once
#include "livechatbase.h"

namespace QtTube
{
    struct LiveChat : LiveChatBase
    {
        QList<LiveChatItem> items;
        QString restrictedMessage;
    };
}
