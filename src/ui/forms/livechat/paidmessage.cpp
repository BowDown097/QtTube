#include "paidmessage.h"
#include "http.h"
#include "innertube/objects/innertubestring.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/httputils.h"
#include "utils/uiutils.h"
#include <QBoxLayout>

constexpr QLatin1String HeaderStylesheet(R"(
    background: #%1;
    border-top: 1px solid transparent;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    color: #%2;
)");

constexpr QLatin1String MessageStylesheet(R"(
    background: #%1;
    border-bottom: 1px solid transparent;
    border-bottom-left-radius: 4px;
    border-bottom-right-radius: 4px;
    color: #%2;
)");

PaidMessage::PaidMessage(const QJsonValue& renderer, QWidget* parent)
    : QWidget(parent),
      amountLabel(new TubeLabel(renderer["purchaseAmountText"]["simpleText"].toString(), this)),
      authorIcon(new QLabel(this)),
      authorLabel(new TubeLabel(renderer["authorName"]["simpleText"].toString(), this)),
      header(new QWidget(this)),
      headerLayout(new QHBoxLayout(header)),
      innerHeaderLayout(new QVBoxLayout),
      layout(new QVBoxLayout(this)),
      messageLabel(new TubeLabel(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    header->setAutoFillBackground(true);
    header->setStyleSheet(HeaderStylesheet
        .arg(QString::number(renderer["headerBackgroundColor"].toVariant().toLongLong(), 16),
             QString::number(renderer["headerTextColor"].toVariant().toLongLong(), 16)));
    layout->addWidget(header);

    headerLayout->setContentsMargins(5, 0, 0, 0);
    headerLayout->setSpacing(0);

    authorIcon->setFixedSize(32, 32);
    authorIcon->setScaledContents(true);
    headerLayout->addWidget(authorIcon);
    headerLayout->addSpacerItem(new QSpacerItem(6, 0));

    HttpReply* iconReply = HttpUtils::cachedInstance().get(renderer["authorPhoto"]["thumbnails"][0]["url"].toString());
    connect(iconReply, &HttpReply::finished, this, &PaidMessage::setAuthorIcon);

    innerHeaderLayout->setContentsMargins(0, 0, 0, 0);
    innerHeaderLayout->setSpacing(0);
    headerLayout->addLayout(innerHeaderLayout);

    authorLabel->setWordWrap(true);
    innerHeaderLayout->addWidget(authorLabel);

    amountLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    amountLabel->setWordWrap(true);
    innerHeaderLayout->addWidget(amountLabel);

    InnertubeObjects::InnertubeString message(renderer["message"]);
    if (message.text.isEmpty())
        return;

    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setAutoFillBackground(true);
    messageLabel->setStyleSheet(MessageStylesheet
        .arg(QString::number(renderer["bodyBackgroundColor"].toVariant().toLongLong(), 16),
             QString::number(renderer["bodyTextColor"].toVariant().toLongLong(), 16)));
    messageLabel->setText(message.text);
    messageLabel->setWordWrap(true);
    layout->addWidget(messageLabel);
}

void PaidMessage::setAuthorIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    authorIcon->setPixmap(UIUtils::pixmapRounded(pixmap, 9999, 9999));
}
