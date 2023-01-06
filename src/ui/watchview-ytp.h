#ifndef USEMPV
#ifndef WATCHVIEWYTP_H
#define WATCHVIEWYTP_H
#include "tubelabel.h"
#include "webengineplayer.h"
#include <QStackedWidget>
#include <QVBoxLayout>

class WatchView : public QWidget
{
    Q_OBJECT
public:
    static WatchView* instance();
    void initialize(QStackedWidget* stackedWidget) { this->stackedWidget = stackedWidget; }
    void loadVideo(const QString& videoId, int progress = 0);
public slots:
    void goBack();
private:
    TubeLabel* channelIcon = nullptr;
    TubeLabel* channelName = nullptr;
    QVBoxLayout* pageLayout = nullptr;
    QHBoxLayout* primaryInfoHbox = nullptr;
    QVBoxLayout* primaryInfoVbox = nullptr;
    QWidget* primaryInfoWrapper = nullptr;
    QStackedWidget* stackedWidget = nullptr;
    TubeLabel* subscribersLabel = nullptr; // TODO: make into Hitchhiker-like subscribe button
    TubeLabel* titleLabel = nullptr;
    WebEnginePlayer* wePlayer = nullptr;

    explicit WatchView(QWidget* parent = nullptr) : QWidget(parent) {}
    void resizeEvent(QResizeEvent*) override; // webengine views don't resize automatically
};

#endif // WATCHVIEWYTP_H
#endif // USEMPV
