#pragma once
#include <QFuture>
#include <QString>

namespace TubeUtils
{
    QFuture<std::pair<QString, bool>> getSubCount(const QString& channelId, const QString& fallback = {});
    QString getUcidFromUrl(const QString& url);
}
