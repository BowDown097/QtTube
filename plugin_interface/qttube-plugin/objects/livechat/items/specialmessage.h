#pragma once
#include <QFont>
#include <QString>

namespace QtTube
{
    struct SpecialMessage
    {
        QString backgroundColor;
        QString content;
        QFont::Style contentStyle = QFont::StyleItalic;
        QString header;
        QFont::Style headerStyle = QFont::StyleNormal;
    };
}
