#pragma once
#include "qttube-plugin/objects/video.h"
#include "ui/views/preloaddata.h"
#include <QWidget>

class ChannelLabel;
class HttpReply;
struct PluginData;
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

    explicit VideoRenderer(PluginData* plugin, QWidget* parent = nullptr);
    void setData(const QtTubePlugin::Video& video);
private:
    PluginData* plugin;
    int progress{};
    QString videoId;
    std::unique_ptr<PreloadData::WatchView> watchPreloadData;
private slots:
    void copyVideoUrl();
    void navigate();
    void showTitleContextMenu(const QPoint& pos);
};
