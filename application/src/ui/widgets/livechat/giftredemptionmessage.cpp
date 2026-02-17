#include "giftredemptionmessage.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

GiftRedemptionMessage::GiftRedemptionMessage(const QtTubePlugin::GiftRedemptionMessage& data, QWidget* parent)
    : QWidget(parent),
      m_authorLabel(new TubeLabel(data.authorName, this)),
      m_layout(new QHBoxLayout(this)),
      m_messageLabel(new TubeLabel(" " + data.content, this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_authorLabel->setStyleSheet("font-weight: bold; color: #2ba640");
    m_layout->addWidget(m_authorLabel);

    m_messageLabel->setFixedWidth(parent->width() - 40);
    m_messageLabel->setFont(QFont(font().toString(), -1, -1, true));
    m_messageLabel->setWordWrap(true);
    m_layout->addWidget(m_messageLabel);
}
