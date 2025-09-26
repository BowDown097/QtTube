#include "tubelabel.h"
#include "qttube-plugin/utils/httprequest.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include <QStyle>
#include <QStyleOption>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>

TubeLabel::TubeLabel(QWidget* parent) : ClickableWidget<QLabel>(parent)
{
    setMouseTracking(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
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
    else if (alignment() & Qt::AlignRight || text().isRightToLeft() || alignRightStyleRegex.match(styleSheet()).hasMatch())
        rect.moveLeft(width() - rect.width());

    if (alignment() & Qt::AlignVCenter && boundingRect().height() < height())
        rect.moveTop(std::abs((boundingRect().height() / 2) - (height() / 2) - rect.y()));
    else if (alignment() & Qt::AlignBottom && boundingRect().height() < height())
        rect.moveTop(std::abs(boundingRect().height() - height() - rect.y()));

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

    std::unique_ptr<QTextDocument> doc = createTextDocument(text(), textLineWidth());
    int y = 0;

    for (QTextBlock block = doc->firstBlock(); block.isValid(); block = block.next())
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

std::unique_ptr<QTextDocument> TubeLabel::createTextDocument(const QString& text, int textWidth) const
{
    QTextOption opt;
    opt.setWrapMode(wordWrap() ? QTextOption::WordWrap : QTextOption::NoWrap);

    std::unique_ptr<QTextDocument> doc = std::make_unique<QTextDocument>();
    doc->setDefaultFont(font());
    doc->setDefaultTextOption(opt);
    doc->setDocumentMargin(margin());

    if (textFormat() == Qt::RichText || (textFormat() == Qt::AutoText && Qt::mightBeRichText(text)))
        doc->setHtml(text);
    else
        doc->setPlainText(text);

    doc->setTextWidth(textWidth);
    doc->documentLayout(); // why do i have to call this for it to actually lay out bruh

    return doc;
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

// this is essentially a complete reimplementation of
// the original method, with some minor changes as
// QLabelPrivate is not available and was used in some spots.
// original method has problems because we can't give it the raw text </3
int TubeLabel::heightForWidth(int w) const
{
    if (text().isEmpty())
        return QLabel::heightForWidth(w);

    if (minimumWidth() > 0)
        w = std::max(w, minimumWidth());

    QMargins cm = contentsMargins();
    QSize contentsMargin(cm.left() + cm.right(), cm.top() + cm.bottom());

    QRect br;

    int hextra = 2 * margin();
    int vextra = hextra;
    QFontMetrics fm(font());

    int align = QStyle::visualAlignment(layoutDirection(), alignment());
    if (wordWrap())
        align |= Qt::TextWordWrap;

    int m = indent();

    if (m < 0 && frameWidth())
        m = fm.horizontalAdvance(u'x') - margin() * 2;
    if (m > 0)
    {
        if ((align & Qt::AlignLeft) || (align & Qt::AlignRight))
            hextra += m;
        if ((align & Qt::AlignTop) || (align & Qt::AlignBottom))
            vextra += m;
    }

    if (Qt::mightBeRichText(m_rawText))
    {
        std::unique_ptr<QTextDocument> doc = createTextDocument(m_rawText, -1);
        if (align & Qt::TextWordWrap)
        {
            if (w >= 0)
                doc->setTextWidth(std::max(w - hextra - contentsMargin.width(), 0));
            else
                doc->adjustSize();
        }

        QSizeF docSize = doc->size();
        br = QRect(QPoint(0, 0), QSize(std::ceil(docSize.width()), std::ceil(docSize.height())));
    }
    else
    {
        int flags = align & ~(Qt::AlignVCenter | Qt::AlignHCenter);
        if (!QKeySequence::mnemonic(m_rawText).isEmpty())
        {
            flags |= Qt::TextShowMnemonic;
            QStyleOption opt;
            opt.initFrom(this);
            if (!style()->styleHint(QStyle::SH_UnderlineShortcut, &opt, this))
                flags |= Qt::TextHideMnemonic;
        }

        bool tryWidth = (w < 0) && (align & Qt::TextWordWrap);
        if (tryWidth)
            w = std::min(fm.averageCharWidth() * 80, maximumSize().width());
        else if (w < 0)
            w = 2000;

        w -= (hextra + contentsMargin.width());
        br = fm.boundingRect(0, 0, w, 2000, flags, m_rawText);
        if (tryWidth && br.height() < 4 * fm.lineSpacing() && br.width() > w / 2)
            br = fm.boundingRect(0, 0, w / 2, 2000, flags, m_rawText);
        if (tryWidth && br.height() < 2 * fm.lineSpacing() && br.width() > w / 4)
            br = fm.boundingRect(0, 0, w / 4, 2000, flags, m_rawText);
    }

    const QSize contentsSize(br.width() + hextra, br.height() + vextra);
    return (contentsSize + contentsMargin).expandedTo(minimumSize()).height();
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

void TubeLabel::processRemoteImages(QString text, ImageFlags flags)
{
    static QRegularExpression imgTagRegex(R"~(<img[^>]+src="((?:https?:)?//[^"]+)")~");
    QRegularExpressionMatchIterator it = imgTagRegex.globalMatch(text);

    m_remoteImageDataMap.clear();
    m_remoteImageReplyMap.clear();

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QString url = match.captured(1);
        if (url.startsWith("//"))
            url.prepend("https:");

        HttpReply* reply = HttpRequest().withDiskCache(qtTubeApp->settings().imageCaching && flags & Cached).get(url);
        connect(reply, &HttpReply::finished, this, std::bind_front(&TubeLabel::remoteImageDownloaded, this, text));
        m_remoteImageReplyMap[reply] = std::move(match);
    }
}

void TubeLabel::remoteImageDownloaded(QString text, const HttpReply& reply)
{
    QByteArray mimeType = reply.header(QNetworkRequest::ContentTypeHeader);
    if (mimeType.isEmpty()) // use image/png as fallback, i guess
        mimeType = "image/png";

    auto it = m_remoteImageReplyMap.find(&reply);
    m_remoteImageDataMap.emplaceBack(
        std::move(it->second), QStringLiteral("data:%1;base64,%2").arg(mimeType, reply.readAll().toBase64()));
    m_remoteImageReplyMap.erase(it);

    if (m_remoteImageReplyMap.empty())
    {
        std::ranges::sort(m_remoteImageDataMap, std::greater{}, [](const auto& p) { return p.first.capturedStart(); });
        for (auto it = m_remoteImageDataMap.begin(); it != m_remoteImageDataMap.end(); it = m_remoteImageDataMap.erase(it))
            text.replace(it->first.capturedStart(1), it->first.capturedLength(1), it->second);
    }

    QLabel::setText(text);
}

void TubeLabel::resizeEvent(QResizeEvent* event)
{
    if (!m_isImage)
        setText(m_rawText);
    else if (m_scaledContents && m_imageFlags & KeepAspectRatio)
        updateMarginsForImageAspectRatio();

    QLabel::resizeEvent(event);
}

void TubeLabel::setImage(const QUrl& url, ImageFlags flags)
{
    if (!url.isValid())
        return;

    m_imageFlags = flags;
    m_isImage = true;
    m_lineRects.clear();
    m_rawText.clear();

    HttpReply* reply = HttpRequest().withDiskCache(qtTubeApp->settings().imageCaching && flags & Cached).get(url);
    connect(reply, &HttpReply::finished, this, &TubeLabel::setImageData);
}

void TubeLabel::setImageData(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.readAll());
    setPixmap(pixmap);
    emit imageSet();
}

void TubeLabel::setMaximumLines(int lines)
{
    m_maximumLines = lines;
    if (!m_rawText.isEmpty())
        setText(m_rawText);
}

void TubeLabel::setPixmap(const QPixmap& pixmap)
{
    m_isImage = true;
    m_lineRects.clear();
    m_rawText.clear();

    if (m_scaledContents)
    {
        if (m_imageFlags & KeepAspectRatio)
        {
            m_imagePixmapHeight = pixmap.height();
            m_imagePixmapWidth = pixmap.width();
            updateMarginsForImageAspectRatio();
        }

        // by default, optimize for fixed size if it is set.
        // this means unsetting the setScaledContents option to disable QLabel's scaling,
        // creating a scaled version of the pixmap ourselves, and then using that.
        // by doing this, the non-scaled pixmap will not be stored in memory.
        if (minimumSize() == maximumSize() && !(m_imageFlags & NoOptimizeForFixedSize))
        {
            QLabel::setScaledContents(false);

            QRect cr = contentsRect();
            cr.adjust(margin(), margin(), margin(), margin());
            const qreal dpr = devicePixelRatio();

            QPixmap scaledPixmap = m_imageFlags & Rounded
                ? UIUtils::pixmapRounded(pixmap)
                      .scaled(cr.size() * dpr, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                : pixmap.scaled(cr.size() * dpr, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            scaledPixmap.setDevicePixelRatio(dpr);

            QLabel::setPixmap(scaledPixmap);
            return;
        }
    }

    QLabel::setPixmap(m_imageFlags & Rounded ? UIUtils::pixmapRounded(pixmap) : pixmap);
}

void TubeLabel::setScaledContents(bool enable)
{
    m_scaledContents = enable;
    QLabel::setScaledContents(enable);
}

void TubeLabel::setText(const QString& text, bool processRemoteImages, ImageFlags remoteImageFlags)
{
    m_isImage = false;
    m_rawText = text;

    if (maximumHeight() == m_calculatedMaximumHeight)
        setMaximumHeight(QWIDGETSIZE_MAX);

    if (text.isEmpty() || m_maximumLines == 0) [[unlikely]]
    {
        QLabel::setText(QString());
        m_lineRects.clear();
        return;
    }

    QFontMetrics fm(font());
    QString outText;

    if (wordWrap())
    {
        QTextLayout textLayout(text, font());
        textLayout.beginLayout();

        int lineNum = 1, y = 0;
        for (QTextLine line = textLayout.createLine(); line.isValid(); line = textLayout.createLine(), ++lineNum)
        {
            line.setLineWidth(textLineWidth());
            y += line.height();

            if (maximumHeight() >= y + line.height() && (m_maximumLines <= 0 || lineNum < m_maximumLines))
            {
                outText.append(text.data() + line.textStart(), line.textLength());
            }
            else
            {
                outText += fm.elidedText(text.mid(line.textStart()), m_elideMode, line.width());
                break;
            }
        }

        textLayout.endLayout();

        if (m_maximumLines > 0)
        {
            m_calculatedMaximumHeight = y;
            setMaximumHeight(m_calculatedMaximumHeight);
        }
    }
    else
    {
        outText = fm.elidedText(text, m_elideMode, textLineWidth());
    }

    QLabel::setText(outText);
    calculateAndSetLineRects();

    if (processRemoteImages && Qt::mightBeRichText(outText))
        TubeLabel::processRemoteImages(outText, remoteImageFlags);
}

int TubeLabel::textLineWidth() const
{
    return maximumWidth() != QWIDGETSIZE_MAX ? std::max(width(), maximumWidth()) : width();
}

void TubeLabel::updateMarginsForImageAspectRatio()
{
    if (m_imagePixmapHeight <= 0 || m_imagePixmapWidth <= 0 || width() <= 0 || height() <= 0)
        return;

    if (width() * m_imagePixmapHeight > height() * m_imagePixmapWidth)
    {
        int m = (width() - (m_imagePixmapWidth * height() / m_imagePixmapHeight)) / 2;
        setContentsMargins(m, 0, m, 0);
    }
    else
    {
        int m = (height() - (m_imagePixmapHeight * width() / m_imagePixmapWidth)) / 2;
        setContentsMargins(0, m, 0, m);
    }
}
