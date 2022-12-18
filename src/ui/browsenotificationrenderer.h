#ifndef BROWSENOTIFICATIONRENDERER_H
#define BROWSENOTIFICATIONRENDERER_H
#include "httpreply.h"
#include "innertube/objects/notification/notification.h"
#include <QHBoxLayout>
#include <QLabel>

class BrowseNotificationRenderer : public QWidget
{
    Q_OBJECT
    QLabel* channelIcon;
    QHBoxLayout* hbox;
    QLabel* sentTimeText;
    QLabel* shortMessage;
    QVBoxLayout* textVbox;
    QLabel* thumbLabel;
public:
    explicit BrowseNotificationRenderer(QWidget* parent);
    void setData(const InnertubeObjects::Notification& notification);
public slots:
    void setChannelIcon(const HttpReply& reply);
    void setThumbnail(const HttpReply& reply);
};

#endif // BROWSENOTIFICATIONRENDERER_H
