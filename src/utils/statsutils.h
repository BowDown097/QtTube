#ifndef STATSUTILS_H
#define STATSUTILS_H

namespace InnertubeEndpoints { class PlayerResponse; }

class Http;
class InnertubeAuthStore;
class InnertubeContext;

namespace StatsUtils
{
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position);
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
}

#endif // STATSUTILS_H
