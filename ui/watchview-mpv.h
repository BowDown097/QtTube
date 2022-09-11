#ifdef USEMPV
#ifndef WATCHVIEWMPV_H
#define WATCHVIEWMPV_H
#include "endpoints/video/player.h"
#include "media/media.h"
#include <QGridLayout>
#include <QListWidget>
#include <QStackedWidget>

class WatchView : public QWidget
{
    Q_OBJECT
public:
    static WatchView* instance();
    void initialize(QStackedWidget* stackedWidget);
    void loadVideo(const InnertubeEndpoints::Player& player);
private slots:
    void mediaStateChanged(Media::State state);
    void volumeChanged(double volume);
private:
    WatchView(QWidget* parent = nullptr);
    QGridLayout* grid;
    Media* media;
    QListWidget* recommendations;
    QStackedWidget* stackedWidget;
};

#endif // WATCHVIEWMPV_H
#endif // USEMPV
