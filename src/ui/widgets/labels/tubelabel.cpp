#include "tubelabel.h"
#include "innertube/objects/innertubestring.h"
#include <QTextLayout>

TubeLabel::TubeLabel(QWidget* parent) : ClickableWidget<QLabel>(parent)
{
    setMouseTracking(true);
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

QRect TubeLabel::alignedRect(const QRect& rect) const
{
    QRect out = rect;

    static QRegularExpression alignCenterStyleRegex(R"(text-align:\s*center)");
    static QRegularExpression alignRightStyleRegex(R"(text-align:\s*right)");

    if (alignment() & Qt::AlignHCenter || alignCenterStyleRegex.match(styleSheet()).hasMatch())
        out.moveLeft((width() - out.width()) / 2);
    else if (alignment() & Qt::AlignRight || alignRightStyleRegex.match(styleSheet()).hasMatch())
        out.moveLeft(width() - out.width());

    return out;
}

QRect TubeLabel::boundingRect() const
{
    QRect out;
    for (const QRect& lineRect : m_lineRects)
        out = out.united(lineRect);
    return out;
}

QRect TubeLabel::boundingRectOfLineAt(const QPoint& point) const
{
    auto it = std::ranges::find_if(m_lineRects, [this, point](const QRect& r) { return alignedRect(r).contains(point); });
    if (it != m_lineRects.end())
        return *it;
    else
        return QRect();
}

void TubeLabel::calculateAndSetLineRects()
{
    m_lineRects.clear();

    QString plainText;
    if (textFormat() == Qt::RichText || (textFormat() == Qt::AutoText && Qt::mightBeRichText(text())))
    {
        QTextDocument doc;
        doc.setHtml(text());
        plainText = doc.toPlainText();
    }
    else
    {
        plainText = text();
    }

    QFontMetrics fm(font());
    if (!wordWrap())
    {
        m_lineRects.append(QRect(0, 0, fm.horizontalAdvance(plainText), fm.height()));
        return;
    }

    int y{};
    QList<QTextLayout*> textLayouts;

    QStringList lines = plainText.split('\n');
    for (const QString& lineText : lines)
    {
        QTextLayout* textLayout = new QTextLayout(lineText, font());
        textLayout->beginLayout();

        forever
        {
            QTextLine line = textLayout->createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(m_hasFixedWidth ? width() : maximumWidth());
            line.setPosition(QPointF(0, y));
            y += lineText.isEmpty() ? fm.height() : line.height();
        }

        textLayout->endLayout();
        textLayouts.append(textLayout);
    }

    int currentLineHeight{};
    for (QTextLayout* textLayout : textLayouts)
    {
        for (int i = 0; i < textLayout->lineCount(); ++i)
        {
            QTextLine line = textLayout->lineAt(i);
            m_lineRects.append(QRect(0, currentLineHeight, line.naturalTextWidth(), line.height()));
            currentLineHeight += line.height();
        }
    }

    qDeleteAll(textLayouts);
}

// enterEvent in ClickableWidget superseded by mouseMoveEvent
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void TubeLabel::enterEvent(QEnterEvent* event)
#else
void TubeLabel::enterEvent(QEvent* event)
#endif
{
    if (boundingRect().isNull())
        ClickableWidget<QLabel>::enterEvent(event);
    else
        QLabel::enterEvent(event);
}

void TubeLabel::leaveEvent(QEvent* event)
{
    unsetCursor();
    ClickableWidget<QLabel>::leaveEvent(event);
}

void TubeLabel::mouseMoveEvent(QMouseEvent* event)
{
    if (boundingRect().isNull())
    {
        QLabel::mouseMoveEvent(event);
        return;
    }

    if (!boundingRectOfLineAt(event->pos()).isNull())
    {
        if (underlineOnHover() && !font().underline())
            makeUnderlined(true);

        if (clickable())
            setCursor(QCursor(Qt::PointingHandCursor));
        else if (cursor().shape() != Qt::PointingHandCursor)
            setCursor(QCursor(Qt::IBeamCursor));
    }
    else
    {
        unsetCursor();
        if (underlineOnHover() && font().underline())
            makeUnderlined(false);
    }

    QLabel::mouseMoveEvent(event);
}

void TubeLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (boundingRect().isNull())
    {
        ClickableWidget<QLabel>::mouseReleaseEvent(event);
        return;
    }

    if (clickable() && event->button() == Qt::LeftButton && !boundingRectOfLineAt(event->pos()).isNull())
        emit clicked();

    QLabel::mouseReleaseEvent(event); // clazy:exclude=skipped-base-method
}

void TubeLabel::setFixedSize(const QSize& size)
{
    QLabel::setFixedSize(size);
    m_hasFixedWidth = true;
}

void TubeLabel::setFixedSize(int width, int height)
{
    setFixedSize(QSize(width, height));
}

void TubeLabel::setFixedWidth(int width)
{
    QLabel::setFixedWidth(width);
    m_hasFixedWidth = true;
}

void TubeLabel::setText(const QString& text)
{
    if (text.isEmpty())
    {
        QLabel::setText(QString());
        m_lineRects.clear();
        return;
    }

    QFontMetrics fm(font());
    if (!wordWrap())
    {
        QLabel::setText(fm.elidedText(text, m_elideMode, m_hasFixedWidth ? width() : maximumWidth()));
        calculateAndSetLineRects();
        return;
    }

    QTextLayout textLayout(text, font());
    textLayout.beginLayout();

    QString outText;
    int y{};

    forever
    {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(m_hasFixedWidth ? width() : maximumWidth());
        int nextLineY = y + fm.lineSpacing();

        if (maximumHeight() >= nextLineY + fm.lineSpacing())
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
    calculateAndSetLineRects();
}
