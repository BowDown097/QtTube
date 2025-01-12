#pragma once
#include <QWidget>

class ChannelLabel;
class HttpReply;
class IconLabel;
class TubeLabel;

class BasePostRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BasePostRenderer(QWidget* parent = nullptr);
protected:
    TubeLabel* channelIconLabel;
    QString channelId;
    ChannelLabel* channelLabel;
    TubeLabel* contentText;
    IconLabel* dislikeLabel;
    IconLabel* likeLabel;
    QString postId;
    TubeLabel* publishedTimeLabel;
    IconLabel* replyLabel;
protected slots:
    void copyPostUrl();
    void linkActivated(const QString& url);
    void navigateChannel();
    void setChannelIcon(const HttpReply& reply);
    void showPublishedTimeContextMenu(const QPoint& pos);
};
