#include "emojistore.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

constexpr QLatin1String EmojiMetadataUrl("https://cdn.jsdelivr.net/gh/googlefonts/emoji-metadata@main/emoji_15_0_ordering.json");

EmojiStore::EmojiStore(QObject* parent)
    : QObject(parent), m_networkAccessManager(new QNetworkAccessManager(this))
{
    m_networkAccessManager->setAutoDeleteReplies(true);

    QNetworkReply* reply = m_networkAccessManager->get(QNetworkRequest(QUrl(EmojiMetadataUrl)));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isArray())
        {
            qCritical() << "Failed to get built-in emoji data. Built-in emojis will not work.";
            m_hasBuiltinEmojis = true;
            return;
        }

        const QJsonArray data = doc.array();
        for (const QJsonValue& groupData : data)
        {
            EmojiGroup group = { .builtin = true, .name = groupData["group"].toString() };
            const QJsonArray emojis = groupData["emoji"].toArray();

            for (const QJsonValue& emojiData : emojis)
            {
                const QJsonArray base = emojiData["base"].toArray();
                if (base.isEmpty())
                    continue;

                QtTubePlugin::Emoji emoji;

                const QJsonArray emoticons = emojiData["emoticons"].toArray();
                for (const QJsonValue& emoticon : emoticons)
                    emoji.emoticons.append(emoticon.toString());

                const QJsonArray shortcodes = emojiData["shortcodes"].toArray();
                for (const QJsonValue& shortcode : shortcodes)
                    emoji.shortcodes.append(shortcode.toString());

                QList<char32_t> codepoints;
                for (const QJsonValue& codepoint : base)
                    codepoints.append(codepoint.toInt());

                QStringList hexCodepoints;
                for (char32_t codepoint : std::as_const(codepoints))
                    hexCodepoints.append(QString::number(codepoint, 16).rightJustified(4, '0'));

                emoji.representation = QString::fromUcs4(codepoints.data(), codepoints.size());
                emoji.url = "https://fonts.gstatic.com/s/e/notoemoji/latest/" + hexCodepoints.join('_') + "/32.png";

                group.emojis.append(emoji);
            }

            m_emojiGroups.append(group);
        }

        m_hasBuiltinEmojis = true;
        emit gotBuiltinEmojis();
    });
}

void EmojiStore::add(const QString& group, const QList<QtTubePlugin::Emoji>& emojis, bool mergeIntoGroup)
{
    if (auto it = std::ranges::find(m_emojiGroups, group, &EmojiGroup::name); it != m_emojiGroups.end())
    {
        if (mergeIntoGroup)
            it->emojis.append(emojis);
        else
            it->emojis = emojis;
    }
    else
    {
        m_emojiGroups.emplaceFront(false, group, emojis);
    }
}

QString& EmojiStore::emojize(QString& text) const
{
    constexpr QLatin1String EmojizeFormat("{{{%1||%2||%3}}}");

    for (const EmojiGroup& emojiGroup : m_emojiGroups)
    {
        for (const QtTubePlugin::Emoji& emoji : emojiGroup.emojis)
        {
            for (const QString& shortcode : emoji.shortcodes)
            {
                if (emojiGroup.builtin)
                    text.replace(shortcode, emoji.representation);
                else
                    text.replace(shortcode, EmojizeFormat.arg(emoji.url, emoji.representation, emoji.shortcodes.front()));
            }
        }
    }

    return text;
}
