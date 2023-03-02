#ifndef WEBCHANNELINTERFACE_H
#define WEBCHANNELINTERFACE_H
#include <QObject>

class WebChannelInterface : public QObject
{
    Q_OBJECT
public:
    static WebChannelInterface* instance();
public slots:
    void switchWatchViewVideo(const QString& videoId);
private:
    Q_DISABLE_COPY(WebChannelInterface)
    WebChannelInterface() = default;
    static inline WebChannelInterface* m_interface;
};

#endif // WEBCHANNELINTERFACE_H
