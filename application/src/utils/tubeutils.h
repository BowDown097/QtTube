#pragma once
#include <QFuture>
#include <QString>

namespace InnertubeEndpoints { struct PlayerResponse; }

class Http;
class InnertubeAuthStore;
struct InnertubeContext;

namespace TubeUtils
{
    QFuture<std::pair<QString, bool>> getSubCount(const QString& channelId, const QString& fallback = {});
    QString getUcidFromUrl(const QString& url);
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void setNeededHeaders(Http& http, InnertubeContext* context, InnertubeAuthStore* authStore);
}
