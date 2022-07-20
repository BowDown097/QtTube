#ifndef INNERTUBEUSERCONFIG_H
#define INNERTUBEUSERCONFIG_H
#include <QJsonObject>

class InnertubeUserConfig
{
public:
    bool lockedSafetyMode;
    InnertubeUserConfig(bool lsm = false) : lockedSafetyMode(lsm) {}
    QJsonObject toJson() const { return {{ "lockedSafetyMode", lockedSafetyMode }}; }
};

#endif // INNERTUBEUSERCONFIG_H
