#ifndef BROWSENOTIFICATIONRENDERER_H
#define BROWSENOTIFICATIONRENDERER_H
#include <QWidget>

namespace InnertubeObjects { class Notification; }

class HttpReply;
class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class BrowseNotificationRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseNotificationRenderer(QWidget* parent);
    void setData(const InnertubeObjects::Notification& notification);
public slots:
    void setChannelIcon(const HttpReply& reply);
    void setThumbnail(const HttpReply& reply);
private:
    TubeLabel* channelIcon;
    QHBoxLayout* hbox;
    TubeLabel* sentTimeText;
    TubeLabel* shortMessage;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
};

#endif // BROWSENOTIFICATIONRENDERER_H
