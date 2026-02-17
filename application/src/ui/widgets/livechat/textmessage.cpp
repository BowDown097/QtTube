#include "textmessage.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

TextMessage::TextMessage(const QtTubePlugin::TextMessage& data, QWidget* parent)
    : QWidget(parent),
      m_authorIcon(new TubeLabel(this)),
      m_authorLabel(new TubeLabel(data.authorName, this)),
      m_contentLayout(new QVBoxLayout),
      m_headerLayout(new QHBoxLayout),
      m_layout(new QHBoxLayout(this)),
      m_messageLabel(new TubeLabel(this)),
      m_timestampLabel(new TubeLabel(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_authorIcon->setFixedSize(32, 32);
    m_authorIcon->setImage(data.authorAvatarUrl, TubeLabel::Cached | TubeLabel::Rounded);
    m_authorIcon->setScaledContents(true);
    m_layout->addWidget(m_authorIcon);
    m_layout->addSpacerItem(new QSpacerItem(6, 0));

    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    m_contentLayout->addStretch();
    m_layout->addLayout(m_contentLayout);

    m_authorLabel->setMaximumWidth(parent->width() - 150);
    m_authorLabel->setStyleSheet(!data.authorNameColor.isEmpty()
        ? QStringLiteral("font-weight: bold; color: %1").arg(data.authorNameColor)
        : QStringLiteral("font-weight: bold"));
    m_headerLayout->addWidget(m_authorLabel);

    m_timestampLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    m_timestampLabel->setText(data.timestampText);
    m_headerLayout->addWidget(m_timestampLabel);

    m_headerLayout->addStretch();
    m_headerLayout->setSpacing(5);
    m_contentLayout->addLayout(m_headerLayout);

    if (!data.content.isEmpty())
    {
        m_messageLabel->setFixedWidth(parent->width() - 70);
        m_messageLabel->setTextFormat(Qt::RichText);
        m_messageLabel->setWordWrap(true);
        m_messageLabel->setText(data.content, true, TubeLabel::Cached);
        m_contentLayout->addWidget(m_messageLabel);
    }
}
