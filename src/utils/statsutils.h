#pragma once

namespace InnertubeEndpoints { struct PlayerResponse; }

class Http;
class InnertubeAuthStore;
struct InnertubeContext;

namespace StatsUtils
{
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position);
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
}
