#pragma once
#include <QFont>
#include <QString>

namespace QtTubePlugin
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
