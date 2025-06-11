#pragma once
#include "innertube/objects/dynamictext.h"
#include "innertube/objects/innertubestring.h"
#include <QMap>

namespace InnertubeEndpoints { struct PlayerResponse; }

namespace TubeUtils
{
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    InnertubeObjects::InnertubeString unattribute(const InnertubeObjects::DynamicText& attributedDescription);
}
