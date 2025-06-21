#include "giftredemptionmessage.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

GiftRedemptionMessage::GiftRedemptionMessage(const QtTube::GiftRedemptionMessage& data, QWidget* parent)
    : QWidget(parent),
      authorLabel(new TubeLabel(data.authorName, this)),
      layout(new QHBoxLayout(this)),
      messageLabel(new TubeLabel(" " + data.content, this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    authorLabel->setStyleSheet("font-weight: bold; color: #2ba640");
    layout->addWidget(authorLabel);

    messageLabel->setFixedWidth(parent->width() - 40);
    messageLabel->setFont(QFont(font().toString(), -1, -1, true));
    messageLabel->setWordWrap(true);
    layout->addWidget(messageLabel);
}
