#pragma once
#include <QList>
#include <QString>

namespace QtTubePlugin
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
