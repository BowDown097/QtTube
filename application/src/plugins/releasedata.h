#pragma once
#include <QDateTime>

struct ReleaseData
{
    struct Asset
    {
        QString downloadUrl;
        QString name;
        QDateTime updatedAt;
    };

    std::optional<Asset> asset;
    QString defaultBranch;
    QString fullName;
    bool isNightly{};
};