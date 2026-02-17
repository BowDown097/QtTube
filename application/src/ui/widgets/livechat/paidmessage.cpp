#include "paidmessage.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

constexpr QLatin1String HeaderStylesheet(R"(
    background: %1;
    border-top: 1px solid transparent;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    color: %2;
)");

constexpr QLatin1String MessageStylesheet(R"(
    background: %1;
    border-bottom: 1px solid transparent;
    border-bottom-left-radius: 4px;
    border-bottom-right-radius: 4px;
    color: %2;
)");

PaidMessage::PaidMessage(const QtTubePlugin::PaidMessage& data, QWidget* parent)
    : QWidget(parent),
      m_amountLabel(new TubeLabel(data.paidAmountText, this)),
      m_authorIcon(new TubeLabel(this)),
      m_authorLabel(new TubeLabel(data.authorName, this)),
      m_header(new QWidget(this)),
      m_headerLayout(new QHBoxLayout(m_header)),
      m_innerHeaderLayout(new QVBoxLayout),
      m_layout(new QVBoxLayout(this)),
      m_messageLabel(new TubeLabel(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_header->setAutoFillBackground(true);
    m_header->setStyleSheet(HeaderStylesheet.arg(data.headerBackgroundColor, data.headerTextColor));
    m_layout->addWidget(m_header);

    m_headerLayout->setContentsMargins(5, 0, 0, 0);
    m_headerLayout->setSpacing(0);

    m_authorIcon->setFixedSize(32, 32);
    m_authorIcon->setImage(data.authorAvatarUrl, TubeLabel::Cached | TubeLabel::Rounded);
    m_authorIcon->setScaledContents(true);
    m_headerLayout->addWidget(m_authorIcon);
    m_headerLayout->addSpacerItem(new QSpacerItem(6, 0));

    m_innerHeaderLayout->setContentsMargins(0, 0, 0, 0);
    m_innerHeaderLayout->setSpacing(0);
    m_headerLayout->addLayout(m_innerHeaderLayout);

    m_authorLabel->setWordWrap(true);
    m_innerHeaderLayout->addWidget(m_authorLabel);

    m_amountLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    m_amountLabel->setWordWrap(true);
    m_innerHeaderLayout->addWidget(m_amountLabel);

    if (!data.content.isEmpty())
    {
        m_messageLabel->setAlignment(Qt::AlignCenter);
        m_messageLabel->setAutoFillBackground(true);
        m_messageLabel->setStyleSheet(MessageStylesheet.arg(data.contentBackgroundColor, data.contentTextColor));
        m_messageLabel->setTextFormat(Qt::RichText);
        m_messageLabel->setWordWrap(true);
        m_messageLabel->setText(data.content, true, TubeLabel::Cached);
        m_layout->addWidget(m_messageLabel);
    }
}
