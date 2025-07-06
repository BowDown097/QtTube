#pragma once
#include <QWidget>

namespace QtTubePlugin { struct Notification; }

class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class BrowseNotificationRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseNotificationRenderer(QWidget* parent = nullptr);
    void setData(const QtTubePlugin::Notification& notification);
private:
    TubeLabel* bodyLabel;
    TubeLabel* channelIconLabel;
    QHBoxLayout* hbox;
    TubeLabel* sentTimeLabel;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
};
