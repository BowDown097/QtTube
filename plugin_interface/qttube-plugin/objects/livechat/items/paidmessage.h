#pragma once
#include <QString>

namespace QtTubePlugin
{
    struct PaidMessage
    {
        QString authorAvatarUrl;
        QString authorName;
        QString content;
        QString contentBackgroundColor;
        QString contentTextColor;
        QString headerBackgroundColor;
        QString headerTextColor;
        QString paidAmountText;
    };
}
