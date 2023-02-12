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
    void hotLoadChannel(const QString& channelId);
    void loadChannel(const QString& channelId);
public slots:
    void clear();
    void goBack();
private:
    ChannelView() {}
    static inline ChannelView* m_channelView;

    struct Rgb
    {
        int r;
        int g;
        int b;
    };

    QLabel* channelBanner;
    QHBoxLayout* channelHeader;
    QWidget* channelHeaderWidget;
    QLabel* channelIcon;
    QString channelId;
    TubeLabel* channelName;
    QTabWidget* channelTabs;
    TubeLabel* handleAndVideos;
    QHBoxLayout* metaHbox;
    QVBoxLayout* metaVbox;
    QVBoxLayout* pageLayout;
    QHBoxLayout* subscribeHbox;
    TubeLabel* subscribersLabel;
    SubscribeWidget* subscribeWidget;

    int getDominant(const QList<int>& arr);
    Rgb getDominantRgb(const QImage& img);
    void setSubscriberCount(const InnertubeEndpoints::ChannelResponse& channelResp);
    void setTabsAndStyles(const InnertubeEndpoints::ChannelResponse& channelResp);
private slots:
    void setBanner(const HttpReply& reply);
    void setIcon(const HttpReply& reply);
};

#endif // CHANNELVIEW_H
