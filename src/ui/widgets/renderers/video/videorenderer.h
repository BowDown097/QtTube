#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H
#include "httpreply.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/elidedtubelabel.h"

class VideoRenderer : public QWidget
{
public:
    explicit VideoRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Reel& reel);
    void setData(const InnertubeObjects::Video& video);
    void setTargetElisionWidth(int width) { targetElisionWidth = width; }
    void setThumbnail(const QString& url);
    void setThumbnailSize(const QSize& size);
private slots:
    void copyChannelUrl();
    void copyDirectUrl();
    void copyVideoUrl();
    void navigateChannel();
    void navigateVideo();
    void setDeArrowData(const HttpReply& reply, const QString& fallbackThumbUrl);
    void setThumbnailData(const HttpReply& reply);
    void showChannelContextMenu(const QPoint& pos);
    void showTitleContextMenu(const QPoint& pos);
protected:
    ChannelLabel* channelLabel;
    TubeLabel* metadataLabel;
    TubeLabel* thumbLabel;
    ElidedTubeLabel* titleLabel;
private:
    QString channelId;
    int progress = 0;
    int targetElisionWidth = 0;
    QSize thumbnailSize;
    QString videoId;
};

#endif // VIDEORENDERER_H
