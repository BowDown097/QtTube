#pragma once
#include "qttube-plugin/objects/emoji.h"
#include <QObject>

class QNetworkAccessManager;

struct EmojiGroup
{
    QString name;
    bool builtin{};
    std::vector<std::unique_ptr<QtTubePlugin::Emoji>> emojis;
};

class EmojiStore : public QObject
{
    Q_OBJECT
public:
    static EmojiStore* instance() { static EmojiStore inst; return &inst; }
    explicit EmojiStore(QObject* parent = nullptr);

    void add(const QString& groupName, QList<QtTubePlugin::Emoji> emojis);

    // replace non-builtin emoji shortcodes in text with "{{{[url]||[representation]||[primary shortcode]}}}", helpful for parsing
    QString emojize(const QString& text) const;

    const std::vector<std::unique_ptr<EmojiGroup>>& emojiGroups() const { return m_emojiGroups; }
    bool hasBuiltinEmojis() const { return m_hasBuiltinEmojis; }
private:
    struct ShortcodeMapEntry
    {
        QtTubePlugin::Emoji* emoji;
        EmojiGroup* group;
    };

    std::vector<std::unique_ptr<EmojiGroup>> m_emojiGroups;
    std::unordered_map<QString, ShortcodeMapEntry> m_shortcodeMap;
    bool m_hasBuiltinEmojis{};
signals:
    void gotBuiltinEmojis();
};
