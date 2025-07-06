#pragma once
#include "items/giftredemptionmessage.h"
#include "items/paidmessage.h"
#include "items/specialmessage.h"
#include "items/textmessage.h"
#include <any>
#include <QList>

namespace QtTubePlugin
{
    using LiveChatItem = std::variant<std::monostate, GiftRedemptionMessage, PaidMessage, SpecialMessage, TextMessage>;

    struct LiveChatViewOption
    {
        QString name;
        std::any data;
    };

    struct LiveChatBase
    {
        std::any nextData;
        QList<LiveChatViewOption> viewOptions;
    };
}
