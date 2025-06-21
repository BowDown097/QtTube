#pragma once
#include <QList>
#include <QString>

namespace QtTube
{
    struct Emoji
    {
        bool animated{};
        QStringList emoticons;
        bool hidden{};
        QString representation;
        QStringList shortcodes;
        QString url;
    };
}
