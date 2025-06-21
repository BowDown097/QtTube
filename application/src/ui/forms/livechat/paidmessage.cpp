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

PaidMessage::PaidMessage(const QtTube::PaidMessage& data, QWidget* parent)
    : QWidget(parent),
      amountLabel(new TubeLabel(data.paidAmountText, this)),
      authorIcon(new TubeLabel(this)),
      authorLabel(new TubeLabel(data.authorName, this)),
      header(new QWidget(this)),
      headerLayout(new QHBoxLayout(header)),
      innerHeaderLayout(new QVBoxLayout),
      layout(new QVBoxLayout(this)),
      messageLabel(new TubeLabel(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    header->setAutoFillBackground(true);
    header->setStyleSheet(HeaderStylesheet.arg(data.headerBackgroundColor, data.headerTextColor));
    layout->addWidget(header);

    headerLayout->setContentsMargins(5, 0, 0, 0);
    headerLayout->setSpacing(0);

    authorIcon->setFixedSize(32, 32);
    authorIcon->setImage(data.authorAvatarUrl, TubeLabel::Cached | TubeLabel::Rounded);
    authorIcon->setScaledContents(true);
    headerLayout->addWidget(authorIcon);
    headerLayout->addSpacerItem(new QSpacerItem(6, 0));

    innerHeaderLayout->setContentsMargins(0, 0, 0, 0);
    innerHeaderLayout->setSpacing(0);
    headerLayout->addLayout(innerHeaderLayout);

    authorLabel->setWordWrap(true);
    innerHeaderLayout->addWidget(authorLabel);

    amountLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    amountLabel->setWordWrap(true);
    innerHeaderLayout->addWidget(amountLabel);

    if (!data.content.isEmpty())
    {
        messageLabel->setAlignment(Qt::AlignCenter);
        messageLabel->setAutoFillBackground(true);
        messageLabel->setStyleSheet(MessageStylesheet.arg(data.contentBackgroundColor, data.contentTextColor));
        messageLabel->setTextFormat(Qt::RichText);
        messageLabel->setWordWrap(true);
        messageLabel->setText(data.content, true, TubeLabel::Cached);
        layout->addWidget(messageLabel);
    }
}
