#include "tubelabel.h"
#include "innertube/objects/innertubestring.h"
#include <QTextLayout>

TubeLabel::TubeLabel(QWidget* parent) : ClickableWidget<QLabel>(parent)
{
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
}

TubeLabel::TubeLabel(const InnertubeObjects::InnertubeString& text, QWidget* parent) : TubeLabel(parent)
{
    setText(text.text);
}

TubeLabel::TubeLabel(const QString& text, QWidget* parent) : TubeLabel(parent)
{
    setText(text);
}

void TubeLabel::setText(const QString& text)
{
    if (m_elideMode == Qt::ElideNone || text.isEmpty())
    {
        QLabel::setText(text);
        return;
    }

    QFontMetrics fm(font());

    if (!wordWrap())
    {
        QLabel::setText(fm.horizontalAdvance(text) > maximumWidth()
            ? fm.elidedText(text, m_elideMode, maximumWidth()) : text);
        return;
    }

    int lineSpacing = fm.lineSpacing();
    int textWidth = std::min(maximumWidth(), fm.horizontalAdvance(text));

    QTextLayout textLayout(text, font());
    textLayout.beginLayout();

    QString outText;
    int y{};

    forever
    {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(textWidth);
        int nextLineY = y + lineSpacing;

        if (maximumHeight() >= nextLineY + lineSpacing)
        {
            outText += QStringView(text.mid(line.textStart(), line.textLength()));
            y = nextLineY;
        }
        else
        {
            outText += fm.elidedText(text.mid(line.textStart()), m_elideMode, line.width());
            line = textLayout.createLine();
            break;
        }
    }

    textLayout.endLayout();
    QLabel::setText(outText);
}
