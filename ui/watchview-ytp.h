#ifndef USEMPV
#ifndef WATCHVIEWYTP_H
#define WATCHVIEWYTP_H
#include "endpoints/video/player.h"
#include "webengineplayer.h"
#include <QGridLayout>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>

class WatchView : public QWidget
{
    Q_OBJECT
public:
    static WatchView* instance();
    void initialize(QStackedWidget* stackedWidget);
    void loadVideo(const InnertubeEndpoints::Player& player);
private slots:
    void goBack();
private:
    WatchView(QWidget* parent = nullptr);
    QPushButton* backButton;
    QGridLayout* grid;
    QListWidget* recommendations;
    QStackedWidget* stackedWidget;
    WebEnginePlayer* wePlayer;
};

#endif // WATCHVIEWYTP_H
#endif // USEMPV
