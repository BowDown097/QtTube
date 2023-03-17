#ifndef WEBCHANNELINTERFACE_H
#define WEBCHANNELINTERFACE_H
#include <QObject>

class WebChannelInterface : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WebChannelInterface)
public:
    static WebChannelInterface* instance();
public slots:
    void switchWatchViewVideo(const QString& videoId);
private:
    WebChannelInterface() = default;
    static inline WebChannelInterface* m_interface;
};

#endif // WEBCHANNELINTERFACE_H
