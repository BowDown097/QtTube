#pragma once
#include <QString>

class QJsonArray;

namespace StringUtils
{
#ifdef QTTUBE_HAS_ICU
    QString condensedNumericString(qint64 num, int precision = 1);
#endif

    QString extractDigits(const QString& str, bool useLocale = true, bool reserve = true);
    QJsonArray makeRichChatMessage(const QString& text);
}
