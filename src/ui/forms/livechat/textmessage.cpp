#include "textmessage.h"
#include "http.h"
#include "innertube/objects/images/responsiveimage.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/httputils.h"
#include "utils/innertubestringformatter.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QJsonArray>

TextMessage::TextMessage(const QJsonValue& renderer, QWidget* parent)
    : QWidget(parent),
      authorIcon(new QLabel(this)),
      authorLabel(new TubeLabel(renderer["authorName"]["simpleText"].toString(), this)),
      contentLayout(new QVBoxLayout),
      headerLayout(new QHBoxLayout),
      layout(new QHBoxLayout(this)),
      messageLabel(new TubeLabel(this)),
      timestampLabel(new TubeLabel(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    authorIcon->setFixedSize(32, 32);
    authorIcon->setScaledContents(true);
    layout->addWidget(authorIcon);
    layout->addSpacerItem(new QSpacerItem(6, 0));

    InnertubeObjects::ResponsiveImage authorPhoto(renderer["authorPhoto"]["thumbnails"]);
    if (const InnertubeObjects::GenericThumbnail* bestPhoto = authorPhoto.bestQuality())
    {
        HttpReply* iconReply = HttpUtils::cachedInstance().get(bestPhoto->url);
        connect(iconReply, &HttpReply::finished, this, &TextMessage::setAuthorIcon);
    }

    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch();
    layout->addLayout(contentLayout);

    authorLabel->setMaximumWidth(parent->width() - 150);

    if (const QJsonArray authorBadges = renderer["authorBadges"].toArray(); !authorBadges.isEmpty())
    {
        bool isModerator = std::ranges::any_of(authorBadges, [](const QJsonValue& badge) {
            return badge["liveChatAuthorBadgeRenderer"]["icon"]["iconType"].toString() == "MODERATOR";
        });
        // if not moderator, assume member (is there anything else?)
        authorLabel->setStyleSheet(isModerator
            ? "font-weight: bold; color: #5e84f1" : "font-weight: bold; color: #2ba640");
    }
    else
    {
        authorLabel->setStyleSheet("font-weight: bold");
    }

    timestampLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    if (const QJsonValue timestampText = renderer["timestampText"]; timestampText.isObject())
    {
        timestampLabel->setText(timestampText.toString());
    }
    else
    {
        quint64 timestampUsec = renderer["timestampUsec"].toString().toULongLong();
        timestampLabel->setText(QDateTime::fromSecsSinceEpoch(timestampUsec / 1000000)
                                    .toString(QLocale::system().timeFormat(QLocale::ShortFormat)));
    }

    headerLayout->addWidget(authorLabel);
    headerLayout->addWidget(timestampLabel);
    headerLayout->addStretch();
    headerLayout->setSpacing(5);
    contentLayout->addLayout(headerLayout);

    messageLabel->setFixedWidth(parent->width() - 70);
    messageLabel->setTextFormat(Qt::RichText);
    messageLabel->setWordWrap(true);
    contentLayout->addWidget(messageLabel);

    InnertubeObjects::InnertubeString message(renderer["message"]);
    InnertubeStringFormatter* fmt = new InnertubeStringFormatter;
    connect(fmt, &InnertubeStringFormatter::finished, fmt, &InnertubeStringFormatter::deleteLater);
    connect(fmt, &InnertubeStringFormatter::readyRead, messageLabel, &TubeLabel::setText);
    fmt->setData(message, false);
}

void TextMessage::setAuthorIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    authorIcon->setPixmap(UIUtils::pixmapRounded(pixmap));
}
