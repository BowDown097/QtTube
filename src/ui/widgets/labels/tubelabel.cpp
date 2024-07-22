#include "tubelabel.h"
#include "innertube/objects/innertubestring.h"
#include <QTextBlock>
#include <QTextDocument>
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

QRect TubeLabel::alignedRect(QRect rect) const
{
    static QRegularExpression alignCenterStyleRegex(R"(text-align:\s*center)");
    static QRegularExpression alignRightStyleRegex(R"(text-align:\s*right)");

    if (alignment() & Qt::AlignHCenter || alignCenterStyleRegex.match(styleSheet()).hasMatch())
        rect.moveLeft((width() - rect.width()) / 2);
    else if (alignment() & Qt::AlignRight || alignRightStyleRegex.match(styleSheet()).hasMatch())
        rect.moveLeft(width() - rect.width());

    return rect;
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
    return it != m_lineRects.end() ? *it : QRect();
}

void TubeLabel::calculateAndSetLineRects()
{
    m_lineRects.clear();

    QTextOption opt;
    opt.setWrapMode(wordWrap() ? QTextOption::WordWrap : QTextOption::NoWrap);

    QTextDocument doc;
    doc.setDefaultFont(font());
    doc.setDefaultTextOption(opt);
    doc.setDocumentMargin(0);

    if (textFormat() == Qt::RichText || (textFormat() == Qt::AutoText && Qt::mightBeRichText(text())))
        doc.setHtml(text());
    else
        doc.setPlainText(text());

    doc.setTextWidth(m_hasFixedWidth ? width() : maximumWidth());
    doc.documentLayout(); // why do i have to call this for it to actually lay out bruh

    int y = 0;
    for (QTextBlock block = doc.firstBlock(); block.isValid(); block = block.next())
    {
        if (QTextLayout* layout = block.layout())
        {
            for (int i = 0; i < layout->lineCount(); ++i)
            {
                QTextLine line = layout->lineAt(i);
                m_lineRects.append(QRect(0, y, line.naturalTextWidth(), line.height()));
                y += line.height();
            }
        }
    }
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
