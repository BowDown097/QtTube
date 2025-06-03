#pragma once
#include <QFuture>
#include <QString>

namespace InnertubeEndpoints { struct PlayerResponse; }

namespace TubeUtils
{
    QFuture<std::pair<QString, bool>> getSubCount(const QString& channelId, const QString& fallback = {});
    QString getUcidFromUrl(const QString& url);
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
}
