#ifndef WEBCHANNELMETHODS_H
#define WEBCHANNELMETHODS_H
#include <QObject>

class WebChannelMethods : public QObject
{
    Q_OBJECT
public:
    Q_DISABLE_COPY(WebChannelMethods)
    static WebChannelMethods* instance();
public slots:
    void switchWatchViewVideo(const QString& videoId);
private:
    WebChannelMethods(QObject* parent = nullptr) : QObject(parent) {}
    static inline WebChannelMethods* m_methods;
};

#endif // WEBCHANNELMETHODS_H
