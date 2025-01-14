#include "textmessage.h"
#include "http.h"
#include "innertube/objects/images/responsiveimage.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/httputils.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QJsonArray>

constexpr QLatin1String ImgPlaceholder("<img src='data:%1;base64,%2' width='20' height='20'>");

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
    if (renderer["timestampText"].isObject())
    {
        timestampLabel->setText(renderer["timestampText"]["simpleText"].toString());
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

    const QJsonArray runs = renderer["message"]["runs"].toArray();
    for (const QJsonValue& v2 : runs)
    {
        if (v2["emoji"].isObject())
        {
            QString url = v2["emoji"]["image"]["thumbnails"][0]["url"].toString();
            QString placeholder = ImgPlaceholder
                .arg(v2["emoji"]["shortcuts"][0].toString() + v2["emoji"]["searchTerms"][0].toString() + url);
            if (!messageLabel->text().contains(placeholder))
            {
                HttpReply* emojiReply = HttpUtils::cachedInstance().get(url);
                connect(emojiReply, &HttpReply::finished, this,
                    std::bind_front(&TextMessage::insertEmojiIntoMessage, this, placeholder));
            }

            messageLabel->setText(messageLabel->text() + placeholder);
        }
        else if (v2["text"].isString())
        {
            messageLabel->setText(messageLabel->text() + v2["text"].toString());
        }
    }
}

void TextMessage::insertEmojiIntoMessage(const QString& placeholder, const HttpReply& reply)
{
    QString data = ImgPlaceholder.arg(reply.header("content-type"), reply.body().toBase64());
    messageLabel->setText(messageLabel->text().replace(placeholder, data));
}

void TextMessage::setAuthorIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    authorIcon->setPixmap(UIUtils::pixmapRounded(pixmap));
}
