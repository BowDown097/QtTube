#pragma once
#include <QMap>

namespace InnertubeEndpoints { struct PlayerResponse; }

namespace TubeUtils
{
    QMap<QByteArray, QByteArray> getNeededHeaders();
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
}
