#ifndef BROWSEVIDEORENDERER_H
#define BROWSEVIDEORENDERER_H
#include "httpreply.h"
#include "innertube/objects/video/videoowner.h"
#include "tubelabel.h"
#include <QVBoxLayout>

class BrowseVideoRenderer : public QWidget
{
    Q_OBJECT
    QString channelId;
    TubeLabel* channelLabel;
    QHBoxLayout* hbox;
    TubeLabel* metadataLabel;
    int progress;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
    TubeLabel* titleLabel;
    QString videoId;
public:
    explicit BrowseVideoRenderer(QWidget* parent = nullptr);
    void setChannelData(const InnertubeObjects::VideoOwner& owner);
    void setVideoData(const QString& length, const QString& publishedTime, int progress, const QString& title, const QString& videoId,
                      const QString& viewCount);
public slots:
    void setThumbnail(const HttpReply& reply);
private slots:
    void copyChannelUrl();
    void copyDirectUrl();
    void copyVideoUrl();
    void navigateChannel();
    void showChannelContextMenu(const QPoint& pos);
    void showTitleContextMenu(const QPoint& pos);
};

#endif // BROWSEVIDEORENDERER_H
