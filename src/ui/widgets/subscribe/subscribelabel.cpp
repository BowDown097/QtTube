#include "subscribelabel.h"
#include "innertube.h"
#include <QMessageBox>

constexpr const char* subscribeStyle = R"(
    background: red;
    border: solid 1px transparent;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
    color: #fefefe;
)";
constexpr const char* subscribeHoveredStyle = R"(
    background: #d90a17;
    border: solid 1px transparent;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
    color: #fefefe;
)";
constexpr const char* subscribedStyle = R"(
    border: 1px solid #555;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 6px;
)";
constexpr const char* unsubscribeStyle = R"(
    border: 1px solid #555;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 2.5px;
)";

SubscribeLabel::SubscribeLabel(QWidget* parent) : QLabel(parent)
{
    setFixedSize(80, 24);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton)
{
    this->subscribed = subscribeButton.subscribed;
    this->subscribeEndpoint = subscribeButton.onSubscribeEndpoints[0]["subscribeEndpoint"];
    this->subscribeText = subscribeButton.unsubscribedButtonText.text;
    this->subscribedText = subscribeButton.subscribedButtonText.text;
    this->unsubscribeText = subscribeButton.unsubscribeButtonText.text;

    const QJsonValue unsubscribeDialog = subscribeButton.onUnsubscribeEndpoints[0]["signalServiceEndpoint"]["actions"][0]
                                         ["openPopupAction"]["popup"]["confirmDialogRenderer"];
    this->unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text;
    this->unsubscribeEndpoint = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"];

    setStyleSheet(subscribed ? subscribedStyle : subscribeStyle);
    setText(subscribed ? subscribedText : subscribeText);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::SubscribeButtonViewModel& subscribeViewModel,
                                        bool subscribed)
{
    this->subscribed = subscribed;
    this->subscribeEndpoint = subscribeViewModel.subscribeButtonContent.onTapCommand["innertubeCommand"]["subscribeEndpoint"];
    this->subscribeText = subscribeViewModel.subscribeButtonContent.buttonText;
    this->subscribedText = subscribeViewModel.unsubscribeButtonContent.buttonText;
    this->unsubscribeText = "Unsubscribe";

    const QJsonValue unsubscribeDialog = subscribeViewModel.unsubscribeButtonContent.onTapCommand["innertubeCommand"]
                                         ["signalServiceEndpoint"]["actions"][0]["openPopupAction"]
                                         ["popup"]["confirmDialogRenderer"];
    this->unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text;
    this->unsubscribeEndpoint = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"];

    setStyleSheet(subscribed ? subscribedStyle : subscribeStyle);
    setText(subscribed ? subscribedText : subscribeText);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SubscribeLabel::enterEvent(QEnterEvent*)
#else
void SubscribeLabel::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::PointingHandCursor));
    if (subscribed)
    {
        setStyleSheet(unsubscribeStyle);
        setText(unsubscribeText);
    }
    else
    {
        setStyleSheet(subscribeHoveredStyle);
    }
}

void SubscribeLabel::leaveEvent(QEvent*)
{
    setCursor(QCursor());
    if (subscribed)
    {
        setStyleSheet(subscribedStyle);
        setText(subscribedText);
    }
    else
    {
        setStyleSheet(subscribeStyle);
    }
}

void SubscribeLabel::mousePressEvent(QMouseEvent*)
{
    if (!InnerTube::instance()->hasAuthenticated())
    {
        QMessageBox::information(nullptr, "Need to log in", "You must be logged in to subscribe to channels.\nLocal subscriptions are planned, but not implemented.");
        return;
    }

    if (subscribed && QMessageBox::question(nullptr, unsubscribeText, unsubscribeDialogText) == QMessageBox::StandardButton::Yes)
    {
        toggleSubscriptionStatus(subscribeStyle, subscribeText);
        InnerTube::instance()->subscribe(unsubscribeEndpoint, false);
    }
    else if (!subscribed)
    {
        toggleSubscriptionStatus(subscribedStyle, subscribedText);
        InnerTube::instance()->subscribe(subscribeEndpoint, true);
    }
}

void SubscribeLabel::toggleSubscriptionStatus(const QString& styleSheet, const QString& newText)
{
    setStyleSheet(styleSheet);
    setText(newText);
    subscribed = !subscribed;
    emit subscribeStatusChanged(subscribed);
}
