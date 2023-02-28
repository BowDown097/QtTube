#ifndef WEBCHANNELMETHODS_H
#define WEBCHANNELMETHODS_H
#include <QObject>

class WebChannelMethods : public QObject
{
    Q_OBJECT
public:
    static WebChannelMethods* instance();
public slots:
    void switchWatchViewVideo(const QString& videoId);
private:
    Q_DISABLE_COPY(WebChannelMethods)
    WebChannelMethods(QObject* parent = nullptr) : QObject(parent) {}
    static inline WebChannelMethods* m_methods;
};

#endif // WEBCHANNELMETHODS_H
