#pragma once
#include <QMap>

namespace InnertubeEndpoints { struct PlayerResponse; }

namespace TubeUtils
{
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
}
