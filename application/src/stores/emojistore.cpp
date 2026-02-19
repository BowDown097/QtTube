#include "emojistore.h"
#include "qttube-plugin/utils/httprequest.h"
#include <QJsonArray>
#include <QJsonDocument>

const QString EmojiMetadataUrl = QStringLiteral("https://cdn.jsdelivr.net/gh/googlefonts/emoji-metadata@main/emoji_15_0_ordering.json");

EmojiStore::EmojiStore(QObject* parent) : QObject(parent)
{
    HttpReply* reply = HttpRequest().get(EmojiMetadataUrl);
    connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
        const QJsonDocument doc = QJsonDocument::fromJson(reply.readAll());
        if (!doc.isArray())
        {
            qCritical() << "Failed to get built-in emoji data. Built-in emojis will not work.";
            m_hasBuiltinEmojis = true;
            emit gotBuiltinEmojis();
            return;
        }

        const QJsonArray data = doc.array();
        for (const QJsonValue& groupData : data)
        {
            auto group = std::make_unique<EmojiGroup>(groupData["group"].toString(), true);
            const QJsonArray emojis = groupData["emoji"].toArray();

            for (const QJsonValue& emojiData : emojis)
            {
                const QJsonArray base = emojiData["base"].toArray();
                if (base.isEmpty())
                    continue;

                auto emoji = std::make_unique<QtTubePlugin::Emoji>();

                const QJsonArray emoticons = emojiData["emoticons"].toArray();
                for (const QJsonValue& emoticon : emoticons)
                    emoji->emoticons.append(emoticon.toString());

                const QJsonArray shortcodes = emojiData["shortcodes"].toArray();
                for (const QJsonValue& shortcode : shortcodes)
                {
                    QString& stored = emoji->shortcodes.emplaceBack(shortcode.toString());
                    m_shortcodeMap.try_emplace(stored, ShortcodeMapEntry(emoji.get(), group.get()));
                }

                QList<char32_t> codepoints;
                for (const QJsonValue& codepoint : base)
                    codepoints.append(codepoint.toInt());

                QStringList hexCodepoints;
                for (char32_t codepoint : std::as_const(codepoints))
                    hexCodepoints.append(QString::number(codepoint, 16).rightJustified(4, '0'));

                emoji->representation = QString::fromUcs4(codepoints.data(), codepoints.size());
                emoji->url = "https://fonts.gstatic.com/s/e/notoemoji/latest/" + hexCodepoints.join('_') + "/32.png";

                group->emojis.push_back(std::move(emoji));
            }

            m_emojiGroups.push_back(std::move(group));
        }

        m_hasBuiltinEmojis = true;
        emit gotBuiltinEmojis();
    });
}

void EmojiStore::add(const QString& groupName, QList<QtTubePlugin::Emoji> emojis)
{
    auto it = std::ranges::find(m_emojiGroups, groupName, &EmojiGroup::name);
    if (it == m_emojiGroups.end())
    {
        it = m_emojiGroups.insert(
            m_emojiGroups.begin(),
            std::make_unique<EmojiGroup>(groupName));
    }
    else
    {
        (*it)->emojis.clear();
        std::erase_if(m_shortcodeMap, [=](const auto& p) { return p.second.group == it->get(); });
    }

    for (QtTubePlugin::Emoji& emojiData : emojis)
    {
        auto emoji = std::make_unique<QtTubePlugin::Emoji>(std::move(emojiData));
        for (const QString& shortcode : std::as_const(emoji->shortcodes))
            m_shortcodeMap.try_emplace(shortcode, ShortcodeMapEntry(emoji.get(), it->get()));
        (*it)->emojis.push_back(std::move(emoji));
    }
}

QString EmojiStore::emojize(const QString& text) const
{
    static const QString emojizeFormat = QStringLiteral("{{{%1||%2||%3}}}");
    static const QRegularExpression findShortcodesRegex(":([-+\\w]+):");

    QString result(text);
    QRegularExpressionMatchIterator it = findShortcodesRegex.globalMatch(text);
    qsizetype offset = 0;

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        auto emojiIt = m_shortcodeMap.find(match.captured());
        if (emojiIt == m_shortcodeMap.end())
            continue;

        QtTubePlugin::Emoji* emoji = emojiIt->second.emoji;
        QString value = emojiIt->second.group->builtin
            ? emoji->representation
            : emojizeFormat.arg(emoji->url, emoji->representation, emoji->shortcodes.front());

        result.replace(offset + match.capturedStart(), match.capturedLength(), value);
        offset += value.size() - match.capturedLength();
    }

    return result;
}
