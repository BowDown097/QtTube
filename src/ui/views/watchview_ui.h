#ifndef WATCHVIEW_UI_H
#define WATCHVIEW_UI_H
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include <QProgressBar>
#include <QScrollArea>
#include <QSpacerItem>

#ifdef USEMPV
#include "lib/media/media.h"
#else
#include "ui/widgets/webengineplayer/webengineplayer.h"
#endif

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
    QHBoxLayout* primaryInfoHbox;
    QVBoxLayout* primaryInfoVbox;
    QWidget* primaryInfoWrapper;
    QScrollArea* scrollArea;
    TubeLabel* showMoreLabel;
    SubscribeWidget* subscribeWidget;
    TubeLabel* titleLabel;
    QHBoxLayout* topLevelButtons;
    TubeLabel* viewCount;

#ifdef USEMPV
    Media* media;
#else
    WebEnginePlayer* wePlayer;
#endif

    QSize calcPlayerSize(const QSize& maxSize) const;
    void setupUi(QWidget* watchView);
public slots:
    void toggleShowMore();
private:
    const QString likeBarStyle = R"(
    QProgressBar {
        border-radius: 2px;
        background-color: #606060;
    }

    QProgressBar::chunk {
        background-color: #1879c6;
    })";

    void setupDate(QWidget* watchView);
    void setupDescription(QWidget* watchView, const QSize& playerSize);
    void setupFrame(QWidget* watchView);
    void setupMenu(QWidget* watchView, const QSize& playerSize);
    void setupPlayer(QWidget* watchView, const QSize& playerSize);
    void setupPrimaryInfo(QWidget* watchView, const QSize& playerSize);
    void setupTitle(QWidget* watchView, const QSize& playerSize);
#ifdef USEMPV
private slots:
    void mediaStateChanged(Media::State state);
    void volumeChanged(double volume);
#endif
};

namespace Ui
{
    class WatchView : public WatchView_Ui {};
}

QT_END_NAMESPACE

#endif // WATCHVIEW_UI_H
