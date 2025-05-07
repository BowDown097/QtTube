#pragma once
#include <QString>

namespace StringUtils
{
    QString bytesString(double bytes);

#ifdef QTTUBE_HAS_ICU
    QString condensedNumericString(qint64 num, int precision = 1);
#endif

    QString extractDigits(const QString& str, bool useLocale = true, bool reserve = true);
    QString extractPath(const QString& str);
}
