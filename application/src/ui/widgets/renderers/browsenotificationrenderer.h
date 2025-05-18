#pragma once
#include <QWidget>

namespace QtTube { struct PluginNotification; }

class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class BrowseNotificationRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseNotificationRenderer(QWidget* parent = nullptr);
    void setData(const QtTube::PluginNotification& notification);
private:
    TubeLabel* bodyLabel;
    TubeLabel* channelIconLabel;
    QHBoxLayout* hbox;
    TubeLabel* sentTimeLabel;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
};
