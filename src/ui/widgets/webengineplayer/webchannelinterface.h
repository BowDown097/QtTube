#ifndef WEBCHANNELINTERFACE_H
#define WEBCHANNELINTERFACE_H
#include <QObject>

class WebChannelInterface : public QObject
{
    Q_OBJECT
public:
    explicit WebChannelInterface(QObject* parent = nullptr);
    Q_INVOKABLE void emitProgressChanged(double progress, double previousProgress)
    { emit progressChanged(progress, previousProgress); }
public slots:
    void switchWatchViewVideo(const QString& videoId);
signals:
    void progressChanged(double progress, double previousProgress);
};

#endif // WEBCHANNELINTERFACE_H
