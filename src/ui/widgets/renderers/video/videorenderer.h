#pragma once
#include <QWidget>

namespace InnertubeObjects
{
class Reel;
class ResponsiveImage;
class Video;
}

namespace PreloadData { class WatchView; }

class ChannelLabel;
class ElidedTubeLabel;
class HttpReply;
class TubeLabel;
class VideoThumbnailWidget;

class VideoRenderer : public QWidget
{
    Q_OBJECT
public:
    ChannelLabel* channelLabel;
    TubeLabel* metadataLabel;
    VideoThumbnailWidget* thumbnail;
    ElidedTubeLabel* titleLabel;

    explicit VideoRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Reel& reel);
    void setData(const InnertubeObjects::Video& video);
    void setTargetElisionWidth(int width) { targetElisionWidth = width; }
    void setThumbnailSize(const QSize& size);
private:
    QString channelId;
    int progress = 0;
    int targetElisionWidth = 0;
    QString videoId;
    PreloadData::WatchView* watchPreloadData{};

    void setThumbnail(const QString& url);
private slots:
    void copyChannelUrl();
    void copyDirectUrl();
    void copyVideoUrl();
    void elideTitle();
    void navigateChannel();
    void navigateVideo();
    void setDeArrowData(const HttpReply& reply, const QString& fallbackThumbUrl);
    void showChannelContextMenu(const QPoint& pos);
    void showTitleContextMenu(const QPoint& pos);
};
