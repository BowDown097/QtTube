#ifndef BROWSENOTIFICATIONRENDERER_H
#define BROWSENOTIFICATIONRENDERER_H
#include "httpreply.h"
#include "innertube/objects/notification/notification.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QHBoxLayout>

class BrowseNotificationRenderer : public QWidget
{
    Q_OBJECT
    TubeLabel* channelIcon;
    QHBoxLayout* hbox;
    TubeLabel* sentTimeText;
    TubeLabel* shortMessage;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
public:
    explicit BrowseNotificationRenderer(QWidget* parent);
    void setData(const InnertubeObjects::Notification& notification);
public slots:
    void setChannelIcon(const HttpReply& reply);
    void setThumbnail(const HttpReply& reply);
};

#endif // BROWSENOTIFICATIONRENDERER_H
