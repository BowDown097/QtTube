#ifndef QTTUBE_SUBSCRIBEBUTTON_H
#define QTTUBE_SUBSCRIBEBUTTON_H
#include "innertube/objects/channel/subscribebutton.h"
#include <QLabel>

class SubscribeLabel : public QLabel
{
public:
    SubscribeLabel(QWidget* parent = nullptr);
    void setPreferredPalette(const QPalette& pal);
    void setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private:
    QPalette preferredPalette;
    InnertubeObjects::SubscribeButton subscribeButton;

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

#endif // QTTUBE_SUBSCRIBEBUTTON_H
