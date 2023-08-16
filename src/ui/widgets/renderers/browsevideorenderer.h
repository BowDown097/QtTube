#ifndef BROWSEVIDEORENDERER_H
#define BROWSEVIDEORENDERER_H
#include "httpreply.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "ui/widgets/labels/channellabel.h"
#include <QVBoxLayout>

class BrowseVideoRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseVideoRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Reel& reel);
    void setData(const InnertubeObjects::Video& video);
    void setTargetElisionWidth(int width) { targetElisionWidth = width; }
    void setThumbnail(const QString& url);
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
