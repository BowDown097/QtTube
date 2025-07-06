#pragma once
#include "qttube-plugin/components/player/webplayer.h"
#include <QWebEngineUrlRequestInterceptor>

class PlayerInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit PlayerInterceptor(QObject* parent = nullptr) : QWebEngineUrlRequestInterceptor(parent) {}
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
private:
    static bool isTrackingUrl(const QUrl& url);
};

class YouTubePlayer : public QtTubePlugin::WebPlayer
{
public:
    explicit YouTubePlayer(QWidget* parent = nullptr);
    void play(const QString& videoId, int progress) override;
    void seek(int progress) override;
private:
    PlayerInterceptor* m_interceptor;
};
