#ifndef CHANNELVIEW_H
#define CHANNELVIEW_H
#include "httpreply.h"
#include "innertube/responses/browse/channelresponse.h"
#include "ui/widgets/subscribewidget.h"
#include <QStackedWidget>
#include <QTabWidget>

class ChannelView : public QWidget
{
    Q_OBJECT
public:
    static ChannelView* instance();
    void hotLoadChannel(const QString& channelId);
    void loadChannel(const QString& channelId);
public slots:
    void clear();
    void goBack();
private:
    Q_DISABLE_COPY(ChannelView)
    ChannelView() = default;
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
    SubscribeWidget* subscribeWidget;

    int getDominant(int arr[256]);
    Rgb getDominantRgb(const QImage& img);
    void setTabsAndStyles(const InnertubeEndpoints::ChannelResponse& channelResp);
private slots:
    void setBanner(const HttpReply& reply);
    void setIcon(const HttpReply& reply);
};

#endif // CHANNELVIEW_H
