#pragma once
#include <QString>

namespace InnertubeEndpoints { struct PlayerResponse; }

class Http;
class InnertubeAuthStore;
struct InnertubeContext;

namespace TubeUtils
{
    QString getUcidFromUrl(const QString& url);
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
}
