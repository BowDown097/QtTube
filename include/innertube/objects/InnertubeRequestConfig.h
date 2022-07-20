#ifndef INNERTUBEREQUESTCONFIG_H
#define INNERTUBEREQUESTCONFIG_H
#include <QJsonObject>

class InnertubeRequestConfig
{
public:
    bool useSsl;
    InnertubeRequestConfig(bool ssl = true) : useSsl(ssl) {}
    QJsonObject toJson() const { return {{ "useSsl", useSsl }}; }
};

#endif // INNERTUBEREQUESTCONFIG_H
