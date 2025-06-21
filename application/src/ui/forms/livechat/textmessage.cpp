#include "textmessage.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

TextMessage::TextMessage(const QtTube::TextMessage& data, QWidget* parent)
    : QWidget(parent),
      authorIcon(new TubeLabel(this)),
      authorLabel(new TubeLabel(data.authorName, this)),
      contentLayout(new QVBoxLayout),
      headerLayout(new QHBoxLayout),
      layout(new QHBoxLayout(this)),
      messageLabel(new TubeLabel(this)),
      timestampLabel(new TubeLabel(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    authorIcon->setFixedSize(32, 32);
    authorIcon->setImage(data.authorAvatarUrl, TubeLabel::Cached | TubeLabel::Rounded);
    authorIcon->setScaledContents(true);
    layout->addWidget(authorIcon);
    layout->addSpacerItem(new QSpacerItem(6, 0));

    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch();
    layout->addLayout(contentLayout);

    authorLabel->setMaximumWidth(parent->width() - 150);
    authorLabel->setStyleSheet(!data.authorNameColor.isEmpty()
        ? QStringLiteral("font-weight: bold; color: %1").arg(data.authorNameColor)
        : QStringLiteral("font-weight: bold"));
    headerLayout->addWidget(authorLabel);

    timestampLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    timestampLabel->setText(data.timestampText);
    headerLayout->addWidget(timestampLabel);

    headerLayout->addStretch();
    headerLayout->setSpacing(5);
    contentLayout->addLayout(headerLayout);

    if (!data.content.isEmpty())
    {
        messageLabel->setFixedWidth(parent->width() - 70);
        messageLabel->setTextFormat(Qt::RichText);
        messageLabel->setWordWrap(true);
        messageLabel->setText(data.content, true, TubeLabel::Cached);
        contentLayout->addWidget(messageLabel);
    }
}
