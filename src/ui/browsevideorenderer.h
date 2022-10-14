#ifndef BROWSEVIDEORENDERER_H
#define BROWSEVIDEORENDERER_H
#include "clickablelabel.h"
#include "httpreply.h"
#include "objects/video/videoowner.h"
#include <QLabel>
#include <QVBoxLayout>

class BrowseVideoRenderer : public QWidget
{
    Q_OBJECT
    QString channelId;
    ClickableLabel* channelLabel;
    QHBoxLayout* hbox;
    QLabel* metadataLabel;
    int progress;
    QVBoxLayout* textVbox;
    ClickableLabel* thumbLabel;
    ClickableLabel* titleLabel;
    QString videoId;
public:
    explicit BrowseVideoRenderer(QWidget* parent = nullptr);
    void setChannelData(const InnertubeObjects::VideoOwner& owner);
    void setVideoData(QString length, QString publishedTime, int progress, const QString& title, const QString& videoId, const QString& viewCount);
public slots:
    void setThumbnail(const HttpReply& reply);
private slots:
    void navigateChannel();
    void navigateVideo();
};

#endif // BROWSEVIDEORENDERER_H
