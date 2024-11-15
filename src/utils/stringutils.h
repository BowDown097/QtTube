#pragma once
#include <QString>

namespace InnertubeObjects { struct InnertubeString; }

namespace StringUtils
{
#ifdef QTTUBE_HAS_ICU
    QString condensedNumericString(qint64 num, int precision = 1);
#endif

    QString extractDigits(const QString& str, bool useLocale = true, bool reserve = true);
    QString extractPath(const QString& str);
    QString innertubeStringToRichText(const InnertubeObjects::InnertubeString& istr, bool useLinkText);
    void truncateUrlString(QString& url, bool prefix);
}
