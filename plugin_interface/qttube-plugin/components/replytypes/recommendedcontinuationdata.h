#pragma once
#include "qttube-plugin/objects/video.h"
#include <any>

namespace QtTubePlugin
{
    struct RecommendedContinuationData
    {
        std::any nextContinuation;
        QList<QtTubePlugin::Video> videos;
    };
}
