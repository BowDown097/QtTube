#pragma once
#include "qttube-plugin/objects/video.h"
#include "ui/views/preloaddata.h"
#include <QWidget>

class ChannelLabel;
class HttpReply;
class QHBoxLayout;
struct PluginEntry;
class TubeLabel;
class VideoThumbnailWidget;

class VideoRenderer : public QWidget
{
    Q_OBJECT
public:
    QHBoxLayout* badgesLayout;
    ChannelLabel* channelLabel;
    TubeLabel* metadataLabel;
    VideoThumbnailWidget* thumbnail;
    TubeLabel* titleLabel;

    explicit VideoRenderer(PluginEntry* plugin, QWidget* parent = nullptr);
    void setData(const QtTubePlugin::Video& video);
private:
    PluginEntry* m_plugin;
    std::unique_ptr<PreloadData::WatchView> m_preloadData;
    int m_progress{};
    QString m_videoId;
private slots:
    void copyVideoUrl();
    void navigate();
    void showTitleContextMenu(const QPoint& pos);
};
