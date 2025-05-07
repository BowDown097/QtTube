#pragma once
#include "ui/views/preloaddata.h"
#include <QWidget>

namespace InnertubeObjects
{
struct CompactVideo;
struct DisplayAd;
struct LockupViewModel;
struct Reel;
struct ShortsLockupViewModel;
struct Video;
struct VideoDisplayButtonGroup;
}

class ChannelLabel;
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
    TubeLabel* titleLabel;

    explicit VideoRenderer(QWidget* parent = nullptr);

    void setData(const InnertubeObjects::CompactVideo& compactVideo,
                 bool useThumbnailFromData = true);
    void setData(const InnertubeObjects::DisplayAd& displayAd,
                 bool useThumbnailFromData = true);
    void setData(const InnertubeObjects::LockupViewModel& lockup,
                 bool useThumbnailFromData = true);
    void setData(const InnertubeObjects::Reel& reel,
                 bool isInGrid = false, bool useThumbnailFromData = true);
    void setData(const InnertubeObjects::ShortsLockupViewModel& shortsLockup,
                 bool isInGrid = false, bool useThumbnailFromData = true);
    void setData(const InnertubeObjects::Video& video,
                 bool useThumbnailFromData = true);
    void setData(const InnertubeObjects::VideoDisplayButtonGroup& video,
                 bool useThumbnailFromData = true);
private:
    int progress{};
    QJsonValue videoEndpoint;
    QString videoId;
    std::unique_ptr<PreloadData::WatchView> watchPreloadData;

    void setThumbnail(const QString& url);
private slots:
    void copyDirectUrl();
    void copyVideoUrl();
    void navigate();
    void setDeArrowData(const QString& fallbackThumbUrl, const HttpReply& reply);
    void showTitleContextMenu(const QPoint& pos);
};
