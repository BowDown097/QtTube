#ifndef INNERTUBECONFIGINFO_H
#define INNERTUBECONFIGINFO_H
#include <QJsonObject>
#include <QString>

class InnertubeConfigInfo
{
public:
    QString appInstallData;
    InnertubeConfigInfo(const QString& aid = "") : appInstallData(aid) {}
    QJsonObject toJson() const { return {{ "appInstallData", appInstallData }}; }
};

#endif // INNERTUBECONFIGINFO_H
