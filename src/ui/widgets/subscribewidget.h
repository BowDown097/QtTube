#ifndef UISUBSCRIBEBUTTON_H
#define UISUBSCRIBEBUTTON_H
#include "innertube/objects/channel/subscribebutton.h"
#include <QLabel>

class SubscribeWidget : public QLabel
{
public:
    SubscribeWidget(const InnertubeObjects::SubscribeButton& subscribeButton, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
private:
    InnertubeObjects::SubscribeButton subscribeButton;

    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

    const QString subscribeStyle = R"(
    background: red;
    border: solid 1px transparent;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
    color: #fefefe;
    )";
    const QString subscribeHoveredStyle = R"(
    background: #d90a17;
    border: solid 1px transparent;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
    color: #fefefe;
    )";
    const QString subscribedStyle = R"(
    border: 1px solid #333;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 6px;
    )";
    const QString unsubscribeStyle = R"(
    border: 1px solid #333;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 2.5px;
    )";
};

#endif // UISUBSCRIBEBUTTON_H
