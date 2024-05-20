#pragma once
#include <QWidget>

namespace InnertubeObjects { class SubscribeButton; }

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
    void setData(const QString& channelId, const QString& descriptionSnippet, const QString& name,
                 const InnertubeObjects::SubscribeButton& subButton, const QString& subCount, const QString& videoCount);
public slots:
    void setThumbnail(const HttpReply& reply);
private slots:
    void copyChannelUrl();
    void navigateChannel();
    void showContextMenu(const QPoint& pos);
private:
    QString channelId;
    TubeLabel* descriptionLabel;
    QHBoxLayout* hbox;
    TubeLabel* metadataLabel;
    SubscribeWidget* subscribeWidget;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
    TubeLabel* titleLabel;
};
