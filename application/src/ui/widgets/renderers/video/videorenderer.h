#pragma once
#include "ui/views/preloaddata.h"
#include <QWidget>

namespace QtTube { struct PluginVideo; }

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

    void setData(const QtTube::PluginVideo& video);
private:
    int progress{};
    QString videoId;
    std::unique_ptr<PreloadData::WatchView> watchPreloadData;

    void setThumbnail(const QString& url);
private slots:
    void copyVideoUrl();
    void navigate();
    void setDeArrowData(const QString& fallbackThumbUrl, const HttpReply& reply);
    void showTitleContextMenu(const QPoint& pos);
};
