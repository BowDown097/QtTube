#include "ytemoji.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ytemoji::UnicodeEmoji::UnicodeEmoji(const QJsonValue& emojiJson)
    : emojiId(emojiJson["emojiId"].toString()),
      image(emojiJson["image"]["thumbnails"][0]["url"].toString()),
      supportsSkinTone(emojiJson["supportsSkinTone"].toBool())
{
    const QJsonArray searchTermsJson = emojiJson["searchTerms"].toArray();
    for (const QJsonValue& searchTerm : searchTermsJson)
        searchTerms.append(searchTerm.toString());

    const QJsonArray shortcutsJson = emojiJson["shortcuts"].toArray();
    for (const QJsonValue& shortcut : shortcutsJson)
    {
        QString shortcutString = shortcut.toString();
        if (shortcutString.endsWith(':'))
            shortcuts.append(shortcutString);
    }
}

ytemoji* ytemoji::instance()
{
    std::call_once(m_onceFlag, [] { m_instance = new ytemoji; });
    return m_instance;
}

ytemoji::ytemoji()
{
    QFile emojisFile(":/emojis-svg-10.json");
    emojisFile.open(QFile::ReadOnly);

    const QJsonArray doc = QJsonDocument::fromJson(emojisFile.readAll()).array();
    for (const QJsonValue& emoji : doc)
        m_unicodeEmojis.append(UnicodeEmoji(emoji));
}

QString ytemoji::emojize(QString s, bool escape)
{
    for (const UnicodeEmoji& emoji : std::as_const(m_unicodeEmojis))
        for (const QString& shortcut : emoji.shortcuts)
            s.replace(shortcut, emoji.emojiId);
    return s;
}

QJsonArray ytemoji::produceRichText(QString s)
{
    QJsonArray textSegments;

    int i, index = -1;
    int sLen = s.size();

    for (i = 0; i < sLen; i++)
    {
        // check if char is colon and not escaped
        if (s[i] != ':' || (i != 0 && s[i - 1] == '\\'))
            continue;

        if (index == -1 || i - index == 1)
        {
            index = i;
            continue;
        }

        auto it = std::ranges::find_if(m_youtubeEmojis, [index, &s, i](const YouTubeEmoji& e) {
            return e.shortcut == s.mid(index, i - index + 1);
        });
        if (it == m_youtubeEmojis.end())
        {
            index = i;
            continue;
        }

        textSegments.append(QJsonObject { { "text", s.left(index) } });
        textSegments.append(QJsonObject { { "emojiId", it->emojiId } });

        s.remove(0, i + 1);
        sLen -= i + 1;
        i = index = -1;
    }

    if (!s.isEmpty()) // if any remaining text, add it
        textSegments.append(QJsonObject { { "text", s } });

    return textSegments;
}
