#pragma once
#include "watchviewplayer.h"

QT_BEGIN_NAMESPACE

class ChannelLabel;
class IconLabel;
struct PluginData;
class QFrame;
class QHBoxLayout;
class QProgressBar;
class QScrollArea;
class QSpacerItem;
class QVBoxLayout;
class SubscribeWidget;
class TubeLabel;
class WatchNextFeed;

class WatchView_Ui : public QObject
{
    Q_OBJECT
public:
    TubeLabel* channelIcon;
    ChannelLabel* channelLabel;
    TubeLabel* date;
    TubeLabel* description;
    IconLabel* dislikeLabel;
    WatchNextFeed* feed;
    QFrame* frame;
    QVBoxLayout* frameLayout;
    QSpacerItem* infoSpacer;
    QProgressBar* likeBar{};
    QHBoxLayout* likeBarWrapper;
    IconLabel* likeLabel;
    QVBoxLayout* menuVbox;
    QWidget* menuWrapper;
    WatchViewPlayer* player;
    QHBoxLayout* primaryInfoHbox;
    QVBoxLayout* primaryInfoVbox;
    QWidget* primaryInfoWrapper;
    QHBoxLayout* primaryLayout;
    QScrollArea* scrollArea;
    TubeLabel* showMoreLabel;
    SubscribeWidget* subscribeWidget;
    TubeLabel* titleLabel;
    QHBoxLayout* topLevelButtons;
    TubeLabel* viewCount;

    void constructLikeBar();
    void setupUi(QWidget* watchView, PluginData* plugin);
private:
    void setupDate(QWidget* watchView);
    void setupDescription(QWidget* watchView);
    void setupFeed(QWidget* watchView, PluginData* plugin);
    void setupFrame(QWidget* watchView);
    void setupMenu(QWidget* watchView);
    void setupPlayer(QWidget* watchView, PluginData* plugin);
    void setupPrimaryInfo(QWidget* watchView, PluginData* plugin);
    void setupTitle(QWidget* watchView);
public slots:
    void setShowMore(bool enabled);
    void toggleShowMore();
private slots:
    void moveFeed(WatchViewPlayer::ScaleMode scaleMode);
    void scrollValueChanged(int value);
};

namespace Ui
{
    class WatchView : public WatchView_Ui {};
}

QT_END_NAMESPACE
