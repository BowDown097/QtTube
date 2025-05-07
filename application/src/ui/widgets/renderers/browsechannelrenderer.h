#pragma once
#include <QPointer>
#include <QWidget>

namespace InnertubeObjects { struct Channel; struct MetadataBadge; struct SubscribeButton; }

class ChannelLabel;
class HttpReply;
class QHBoxLayout;
class QVBoxLayout;
class SubscribeWidget;
class TubeLabel;

class BrowseChannelRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseChannelRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Channel& channel);
private:
    QString channelId;
    TubeLabel* descriptionLabel;
    QHBoxLayout* hbox;
    QPointer<TubeLabel> metadataLabel;
    SubscribeWidget* subscribeWidget;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
    ChannelLabel* titleLabel;
public slots:
    void setThumbnail(const HttpReply& reply);
private slots:
    void navigateChannel();
};
