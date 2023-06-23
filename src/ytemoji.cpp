#include "ytemoji.h"
#include <QJsonObject>

namespace ytemoji
{
    QString emojize(QString s, bool escape)
    {
        int index = -1;
        int sLen = s.size();

        for (int i = 0; i < sLen; i++)
        {
            // check if char is colon and not escaped
            if (s[i] != ':' || (escape && i != 0 && s[i - 1] == '\\'))
                continue;

            if (index == -1 || i - index == 1)
            {
                index = i;
                continue;
            }

            QMap<QString, QString>::const_iterator it = BUILTIN_EMOJIS.constFind(s.mid(index, i - index + 1));
            if (it == BUILTIN_EMOJIS.constEnd())
            {
                index = i;
                continue;
            }

            const QString& emo = it.value();
            // replace from index to i
            s.replace(index, i - index + 1, emo);
            int goBack = i - index + 1 - emo.size();
            sLen -= goBack;
            i -= goBack;
            index = -1;
        }

        return s;
    }

    QJsonArray produceRichText(QString s)
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

            auto it = std::ranges::find_if(YOUTUBE_EMOJIS, [index, &s, i](const YouTubeEmoji& e) {
                return e.emojiName == s.mid(index, i - index + 1);
            });
            if (it == YOUTUBE_EMOJIS.end())
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
}
