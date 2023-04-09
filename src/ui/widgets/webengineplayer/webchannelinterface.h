#ifndef WEBCHANNELINTERFACE_H
#define WEBCHANNELINTERFACE_H
#include <QObject>

class WebChannelInterface : public QObject
{
    Q_OBJECT
public:
    explicit WebChannelInterface(QObject* parent = nullptr) : QObject(parent) {}
public slots:
    void switchWatchViewVideo(const QString& videoId);
};

#endif // WEBCHANNELINTERFACE_H
