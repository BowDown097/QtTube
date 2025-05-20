#pragma once

namespace InnertubeEndpoints { struct PlayerResponse; }

class Http;
class InnertubeAuthStore;
struct InnertubeContext;

namespace TubeUtils
{
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
}
