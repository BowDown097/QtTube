#pragma once
#include <QString>

namespace QtTubePlugin
{
    enum class ResolveUrlTarget { NotResolved, PlainUrl, Video, Channel, Search };

    struct ResolveUrlData
    {
        bool continuePlayback{};
        QString data;
        QString input;
        ResolveUrlTarget target = ResolveUrlTarget::NotResolved;
        qint64 videoProgress{};
    };
}
