#include "stringutils.h"
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

    QString extractPath(const QString& str)
    {
        QString out;
        quint8 quoteCount{};

        for (QChar c : str)
        {
            if (c == '"' || c == '\'')
                ++quoteCount;
            else if (c.isSpace() && quoteCount != 1)
                break;
            else
                out += c;
        }

        return out;
    }
}
