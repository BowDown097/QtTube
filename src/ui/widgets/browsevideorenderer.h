#ifndef BROWSEVIDEORENDERER_H
#define BROWSEVIDEORENDERER_H
#include "channellabel.h"
#include "httpreply.h"
#include "innertube/objects/video/videoowner.h"
#include <QVBoxLayout>

class BrowseVideoRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseVideoRenderer(QWidget* parent = nullptr);
    void setChannelData(const InnertubeObjects::VideoOwner& owner);
    void setTargetElisionWidth(int width) { targetElisionWidth = width; }
    void setVideoData(const QString& length, const QString& publishedTime, int progress, QString title, const QString& videoId,
                      const QString& viewCount);
public slots:
    void setThumbnail(const HttpReply& reply);
private slots:
    void copyChannelUrl();
    void copyDirectUrl();
    void copyVideoUrl();
    void navigateChannel();
    void navigateVideo();
    void showChannelContextMenu(const QPoint& pos);
    void showTitleContextMenu(const QPoint& pos);
private:
    QString channelId;
    ChannelLabel* channelLabel;
    QHBoxLayout* hbox;
    TubeLabel* metadataLabel;
    int progress;
    int targetElisionWidth;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
    TubeLabel* titleLabel;
    QString videoId;
};

#endif // BROWSEVIDEORENDERER_H
