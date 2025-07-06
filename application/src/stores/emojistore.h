#pragma once
#include "qttube-plugin/objects/emoji.h"
#include <QObject>

class QNetworkAccessManager;

struct EmojiGroup
{
    bool builtin{};
    QString name;
    QList<QtTubePlugin::Emoji> emojis;
};

class EmojiStore : public QObject
{
    Q_OBJECT
public:
    static EmojiStore* instance() { static EmojiStore inst; return &inst; }
    explicit EmojiStore(QObject* parent = nullptr);

    void add(const QString& group, const QList<QtTubePlugin::Emoji>& emojis, bool mergeIntoGroup);

    // replace non-builtin emoji shortcodes in text with "{{{[url]||[representation]||[primary shortcode]}}}", helpful for parsing
    QString& emojize(QString& text) const;

    const QList<EmojiGroup>& emojiGroups() const { return m_emojiGroups; }
    bool hasBuiltinEmojis() const { return m_hasBuiltinEmojis; }
private:
    QList<EmojiGroup> m_emojiGroups;
    bool m_hasBuiltinEmojis{};
    QNetworkAccessManager* m_networkAccessManager;
signals:
    void gotBuiltinEmojis();
};
