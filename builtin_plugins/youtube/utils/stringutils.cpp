#include "stringutils.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>

#ifdef QTTUBE_HAS_ICU
#include <unicode/errorcode.h>
#include <unicode/numberformatter.h>
#endif

namespace StringUtils
{
    // Qt provides no way to do this (surprisingly), so we have to do it manually with ICU.
    #ifdef QTTUBE_HAS_ICU
    QString condensedNumericString(qint64 num, int precision)
    {
        using namespace icu::number;
        static const LocalizedNumberFormatter formatter = NumberFormatter::withLocale(icu::Locale::getDefault())
            .notation(Notation::compactShort()).precision(Precision::maxFraction(1));

        icu::ErrorCode errorCode;
        FormattedNumber formatted = formatter.formatInt(num, errorCode);
        if (errorCode.isFailure())
        {
            qDebug().noquote().nospace() << "Error condensing " << num << ": " << errorCode.errorName();
            return QString();
        }

        icu::UnicodeString formattedStr = formatted.toTempString(errorCode);
        if (errorCode.isFailure())
        {
            qDebug().noquote().nospace() << "Error condensing " << num << ": " << errorCode.errorName();
            return QString();
        }

        return QString(reinterpret_cast<const QChar*>(formattedStr.getBuffer()), formattedStr.length());
    }
    #endif

    QString extractDigits(const QString& str, bool useLocale, bool reserve)
    {
        QString out;
        if (reserve)
            out.reserve(str.size());

        for (QChar c : str)
            if (c.isDigit())
                out.append(c);

        out.squeeze();
        return useLocale ? QLocale::system().toString(out.toLongLong()) : out;
    }

    QJsonArray makeRichChatMessage(const QString& text)
    {
        QJsonArray textSegments;

        static QRegularExpression emojiRegex(R"(\{\{\{(.*?)\|\|(.*?)\|\|(.*?)\}\}\})");
        QRegularExpressionMatchIterator it = emojiRegex.globalMatch(text);
        qsizetype lastIndex = 0;

        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            qsizetype start = match.capturedStart();
            qsizetype end = match.capturedEnd();

            // add preceding text as segment
            if (start > lastIndex)
            {
                QString plainText = text.mid(lastIndex, start - lastIndex);
                textSegments.append(QJsonObject {{ "text", plainText }});
            }

            // add emoji as segment
            textSegments.append(QJsonObject {{ "emojiId", match.captured(2) }});

            lastIndex = end;
        }

        // add any remaining text as segment
        if (lastIndex < text.size())
        {
            QString plainText = text.mid(lastIndex);
            textSegments.append(QJsonObject {{ "text", plainText }});
        }

        return textSegments;
    }
}
