#ifndef QTTUBE_SUBSCRIBEBUTTON_H
#define QTTUBE_SUBSCRIBEBUTTON_H
#include "innertube/objects/channel/subscribebutton.h"
#include "innertube/objects/viewmodels/subscribebuttonviewmodel.h"
#include <QLabel>

class SubscribeLabel : public QLabel
{
    Q_OBJECT
public:
    explicit SubscribeLabel(QWidget* parent = nullptr);
    void setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton);
    void setSubscribeButton(const InnertubeObjects::SubscribeButtonViewModel& subscribeViewModel, bool subscribed);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
signals:
    void subscribeStatusChanged(bool subscribed);
private:
    bool subscribed;
    QJsonValue subscribeEndpoint;
    QString subscribeText;
    QString subscribedText;
    QString unsubscribeDialogText;
    QJsonValue unsubscribeEndpoint;
    QString unsubscribeText;

    void toggleSubscriptionStatus(const QString& styleSheet, const QString& newText);
};

#endif // QTTUBE_SUBSCRIBEBUTTON_H
