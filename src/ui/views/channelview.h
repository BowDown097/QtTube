#ifndef CHANNELVIEW_H
#define CHANNELVIEW_H
#include "httpreply.h"
#include "innertube/responses/browse/channelresponse.h"
#include "ui/widgets/subscribewidget.h"
#include "ui/widgets/tubelabel.h"
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>

class ChannelView : public QWidget
{
    Q_OBJECT
public:
    Q_DISABLE_COPY(ChannelView)
    static ChannelView* instance();
    void loadChannel(const QString& channelId);
public slots:
    void goBack();
private:
    ChannelView() {}
    static inline ChannelView* m_channelView;

    QLabel* channelBanner;
    QHBoxLayout* channelHeader;
    QWidget* channelHeaderWidget;
    QLabel* channelIcon;
    QHBoxLayout* metaHbox;
    QVBoxLayout* metaVbox;
    TubeLabel* channelName;
    QTabWidget* channelTabs;
    TubeLabel* handleAndVideos;
    QVBoxLayout* pageLayout;
    QHBoxLayout* subscribeHbox;
    SubscribeWidget* subscribeWidget;
    TubeLabel* subscribersLabel;

    int getDominant(const QList<int>& arr);
    std::tuple<int, int, int> getDominantRgb(const QImage& img);
    void setSubscriberCount(const InnertubeEndpoints::ChannelResponse& channelResponse);
private slots:
    void setBanner(const HttpReply& reply);
    void setIcon(const HttpReply& reply);
};

#endif // CHANNELVIEW_H
