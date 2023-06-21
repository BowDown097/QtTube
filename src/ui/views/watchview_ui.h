#ifndef WATCHVIEW_UI_H
#define WATCHVIEW_UI_H
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include "watchviewplayer.h"
#include <QProgressBar>
#include <QScrollArea>
#include <QSpacerItem>

QT_BEGIN_NAMESPACE

class WatchView_Ui : public QObject
{
    Q_OBJECT
public:
    TubeLabel* channelIcon;
    ChannelLabel* channelLabel;
    TubeLabel* date;
    TubeLabel* description;
    IconLabel* dislikeLabel;
    QFrame* frame;
    QSpacerItem* infoSpacer;
    QProgressBar* likeBar;
    QHBoxLayout* likeBarWrapper;
    IconLabel* likeLabel;
    QVBoxLayout* menuVbox;
    QWidget* menuWrapper;
    WatchViewPlayer* player;
    QHBoxLayout* primaryInfoHbox;
    QVBoxLayout* primaryInfoVbox;
    QWidget* primaryInfoWrapper;
    QScrollArea* scrollArea;
    TubeLabel* showMoreLabel;
    SubscribeWidget* subscribeWidget;
    TubeLabel* titleLabel;
    QHBoxLayout* topLevelButtons;
    TubeLabel* viewCount;

    void setupUi(QWidget* watchView);
public slots:
    void toggleShowMore();
private:
    void setupDate(QWidget* watchView);
    void setupDescription(QWidget* watchView);
    void setupFrame(QWidget* watchView);
    void setupMenu(QWidget* watchView);
    void setupPlayer(QWidget* watchView);
    void setupPrimaryInfo(QWidget* watchView);
    void setupTitle(QWidget* watchView);
};

namespace Ui
{
    class WatchView : public WatchView_Ui {};
}

QT_END_NAMESPACE

#endif // WATCHVIEW_UI_H
