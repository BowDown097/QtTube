#ifndef STATSUTILS_H
#define STATSUTILS_H
#include "http.h"
#include "innertube/itc-objects/innertubeauthstore.h"
#include "innertube/responses/video/playerresponse.h"

namespace StatsUtils
{
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position);
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
}

#endif // STATSUTILS_H
