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
    static ChannelView* instance();
    void initialize(QStackedWidget* stackedWidget) { this->stackedWidget = stackedWidget; }
    void loadChannel(const QString& channelId);
public slots:
    void goBack();
private:
    QLabel* channelBanner = nullptr;
    QHBoxLayout* channelHeader = nullptr;
    QWidget* channelHeaderWidget = nullptr;
    QLabel* channelIcon = nullptr;
    QHBoxLayout* metaHbox = nullptr;
    QVBoxLayout* metaVbox = nullptr;
    TubeLabel* channelName = nullptr;
    QTabWidget* channelTabs = nullptr;
    TubeLabel* handleAndVideos = nullptr;
    QVBoxLayout* pageLayout = nullptr;
    QStackedWidget* stackedWidget = nullptr;
    QHBoxLayout* subscribeHbox = nullptr;
    SubscribeWidget* subscribeWidget = nullptr;
    TubeLabel* subscribersLabel = nullptr;

    explicit ChannelView(QWidget* parent = nullptr) : QWidget(parent) {}
    int getDominant(const QList<int>& arr);
    std::tuple<int, int, int> getDominantRgb(const QImage& img);
    void setSubscriberCount(const InnertubeEndpoints::ChannelResponse& channelResponse);
private slots:
    void setBanner(const HttpReply& reply);
    void setIcon(const HttpReply& reply);
};

#endif // CHANNELVIEW_H
