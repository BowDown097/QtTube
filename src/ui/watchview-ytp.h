#ifndef USEMPV
#ifndef WATCHVIEWYTP_H
#define WATCHVIEWYTP_H
#include "clickablelabel.h"
#include "innertube/responses/video/nextresponse.h"
#include "innertube/responses/video/playerresponse.h"
#include "webengineplayer.h"
#include <QStackedWidget>
#include <QVBoxLayout>

class WatchView : public QWidget
{
    Q_OBJECT
public:
    static WatchView* instance();
    void initialize(QStackedWidget* stackedWidget) { this->stackedWidget = stackedWidget; }
    void loadVideo(const InnertubeEndpoints::NextResponse& nextResp, const InnertubeEndpoints::PlayerResponse& playerResp, int progress = 0);
public slots:
    void goBack();
private:
    ClickableLabel* channelIcon = nullptr;
    ClickableLabel* channelName = nullptr;
    QVBoxLayout* pageLayout = nullptr;
    QHBoxLayout* primaryInfoHbox = nullptr;
    QVBoxLayout* primaryInfoVbox = nullptr;
    QWidget* primaryInfoWrapper = nullptr;
    QStackedWidget* stackedWidget = nullptr;
    QLabel* subscribersLabel = nullptr; // TODO: make into Hitchhiker-like subscribe button
    QLabel* titleLabel = nullptr;
    WebEnginePlayer* wePlayer = nullptr;

    explicit WatchView(QWidget* parent = nullptr) : QWidget(parent) {}
    void resizeEvent(QResizeEvent*) override; // webengine views don't resize automatically
};

#endif // WATCHVIEWYTP_H
#endif // USEMPV
