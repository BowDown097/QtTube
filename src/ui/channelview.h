#ifndef CHANNELVIEW_H
#define CHANNELVIEW_H
#include <QLabel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>

class ChannelView : public QWidget
{
    Q_OBJECT
public:
    static ChannelView* instance();
    void initialize(QStackedWidget* stackedWidget) { this->stackedWidget = stackedWidget; }
    void loadChannel();
public slots:
    void goBack();
private:
    QLabel* channelBanner = nullptr;
    QHBoxLayout* channelHeader = nullptr;
    QLabel* channelIcon = nullptr;
    QLabel* channelName = nullptr;
    QTabWidget* channelTabs = nullptr;
    QVBoxLayout* pageLayout = nullptr;
    QStackedWidget* stackedWidget = nullptr;
    QLabel* subscriberCount = nullptr;

    explicit ChannelView(QWidget* parent = nullptr) : QWidget(parent) {}
};

#endif // CHANNELVIEW_H
