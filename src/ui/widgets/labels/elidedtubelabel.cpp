#include "elidedtubelabel.h"
#include <QMouseEvent>
#include <QTextLayout>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ElidedTubeLabel::enterEvent(QEnterEvent*)
#else
void ElidedTubeLabel::enterEvent(QEvent*)
#endif
{
    if (m_clickable)
        setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    if (m_underline)
        setStyleSheet("QLabel { text-decoration: underline; }");
}

void ElidedTubeLabel::leaveEvent(QEvent*)
{
    if (m_clickable)
        setCursor(QCursor());
    if (m_underline)
        setStyleSheet(QString());
}

void ElidedTubeLabel::mousePressEvent(QMouseEvent* event)
{
    if (m_clickable && event->button() == Qt::LeftButton)
        emit clicked();
}

void ElidedTubeLabel::setText(const QString& text)
{
    QFontMetrics fm(font());
    int lineSpacing = fm.lineSpacing();
    int y = 0;

    QTextLayout textLayout(text, font());
    textLayout.beginLayout();

    QString outText;
    forever
    {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(width());
        int nextLineY = y + lineSpacing;

        if (maximumHeight() >= nextLineY + lineSpacing)
        {
            outText += QStringView(text.mid(line.textStart(), line.textLength()));
            y = nextLineY;
        }
        else
        {
            QString lastLine = text.mid(line.textStart());
            QString elidedLastLine = fm.elidedText(lastLine, Qt::ElideRight, width());
            outText += elidedLastLine;
            line = textLayout.createLine();
            break;
        }
    }

    textLayout.endLayout();
    QLabel::setText(outText);
}
