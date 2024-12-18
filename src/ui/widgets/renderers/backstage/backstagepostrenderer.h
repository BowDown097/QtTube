#pragma once
#include <QWidget>

namespace InnertubeObjects
{
    struct BackstageImage;
    struct BackstagePost;
    struct Poll;
    struct Video;
}

class ChannelLabel;
class HttpReply;
class IconLabel;
class QHBoxLayout;
class QLabel;
class QVBoxLayout;
class TubeLabel;

class BackstagePostRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BackstagePostRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::BackstagePost& post);
private:
    QHBoxLayout* actionButtons;
    TubeLabel* channelIconLabel;
    QString channelId;
    ChannelLabel* channelLabel;
    QHBoxLayout* channelTimeLayout;
    TubeLabel* contentText;
    IconLabel* dislikeLabel;
    QVBoxLayout* innerLayout;
    QHBoxLayout* layout;
    IconLabel* likeLabel;
    QString postId;
    TubeLabel* publishedTimeLabel;
    TubeLabel* readMoreLabel;
    QString readMoreText;
    IconLabel* replyLabel;
    QString showLessText;
    QString surface;

    void setImage(const InnertubeObjects::BackstageImage& image);
    void setPoll(const InnertubeObjects::Poll& poll);
    void setVideo(const InnertubeObjects::Video& video);
private slots:
    void copyPostUrl();
    void linkActivated(const QString& url);
    void navigateChannel();
    void setChannelIcon(const HttpReply& reply);
    void setImageLabelData(QLabel* imageLabel, const HttpReply& reply);
    void showPublishedTimeContextMenu(const QPoint& pos);
    void toggleReadMore();
signals:
    void dynamicSizeChange(const QSize& newSize);
};
