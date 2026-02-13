#pragma once
#include <QWidget>

namespace QtTubePlugin { struct Notification; }

class QHBoxLayout;
class QVBoxLayout;
struct ReleaseData;
class TubeLabel;

class BrowseNotificationRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseNotificationRenderer(QWidget* parent = nullptr);
    void setData(const QtTubePlugin::Notification& notification);
    void setData(const QString& pluginName, const ReleaseData& data);
private:
    TubeLabel* m_bodyLabel;
    QHBoxLayout* m_buttonsLayout;
    TubeLabel* m_channelIconLabel;
    QVBoxLayout* m_innerLayout;
    QHBoxLayout* m_primaryLayout;
    TubeLabel* m_sentTimeLabel;
    TubeLabel* m_thumbLabel;
};
