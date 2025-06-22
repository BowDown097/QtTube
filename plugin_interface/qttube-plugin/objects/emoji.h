#pragma once
#include <QList>
#include <QString>

namespace QtTube
{
    struct Emoji
    {
        QStringList emoticons;
        bool hidden{};
        QString representation;
        QStringList shortcodes;
        QString url;
    };
}
