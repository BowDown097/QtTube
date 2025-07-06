#include "emojigraphicsitem.h"
#include "httprequest.h"
#include <QBuffer>
#include <QGraphicsSceneMouseEvent>
#include <QImageReader>
#include <QPainter>
#include <QStyleOption>
#include <QThreadPool>
#include <QTimer>

EmojiGraphicsItem::EmojiGraphicsItem(const QtTubePlugin::Emoji& data, QGraphicsItem* parent)
    : QObject(), QGraphicsPixmapItem(parent), m_data(data)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setSize(24, 24);
    setToolTip(data.shortcodes.front());
    setTransformationMode(Qt::SmoothTransformation);

    HttpReply* reply = HttpRequest().withDiskCache(true).get(data.url);
    connect(reply, &HttpReply::finished, this, &EmojiGraphicsItem::setImageData);
}

QRectF EmojiGraphicsItem::boundingRect() const
{
    return m_size.isValid() ? QRectF(offset(), m_size) : QGraphicsPixmapItem::boundingRect();
}

void EmojiGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(Qt::PointingHandCursor);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void EmojiGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    unsetCursor();
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

void EmojiGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QGraphicsPixmapItem::mousePressEvent(event);
}

void EmojiGraphicsItem::nextFrame()
{
    m_currentFrame = (m_currentFrame + 1) % m_frames.size();
    setPixmap(m_frames[m_currentFrame].pixmap);
    m_frameTimer->start(m_frames[m_currentFrame].duration);
}

void EmojiGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform, transformationMode() == Qt::SmoothTransformation);
    painter->drawPixmap(boundingRect(), pixmap(), QRectF());
}

void EmojiGraphicsItem::setImageData(const HttpReply& reply)
{
    QThreadPool::globalInstance()->start([this, data = reply.readAll(), file = reply.url().fileName()] {
        QBuffer buffer;
        buffer.setData(data);

        QImageReader reader(&buffer);
        if (!reader.canRead())
        {
            qWarning() << "Could not read emoji from" << file;
            return;
        }

        int imageCount = reader.imageCount();
        if (reader.size().isEmpty() || imageCount <= 0)
            return;

        m_frames.reserve(imageCount);
        for (int i = 0; i < imageCount; ++i)
        {
            if (QPixmap pixmap = QPixmap::fromImageReader(&reader); !pixmap.isNull())
            {
                int duration = reader.nextImageDelay();
                if (duration <= 10)
                    duration = 100;
                duration = std::max(20, duration);
                m_frames.emplaceBack(duration, pixmap);
            }
        }

        QMetaObject::invokeMethod(this, &EmojiGraphicsItem::updateWithData, Qt::QueuedConnection);
    });
}

// fall back to QGraphicsItem::shape(), as QGraphicsPixmapItem::shape() relies on the full pixmap size rather than bounding rect
QPainterPath EmojiGraphicsItem::shape() const
{
    return QGraphicsItem::shape(); // clazy:exclude=skipped-base-method
}

void EmojiGraphicsItem::updateWithData()
{
    setPixmap(m_frames.front().pixmap);
    if (m_frames.size() > 1)
    {
        m_frameTimer = new QTimer(this);
        connect(m_frameTimer, &QTimer::timeout, this, &EmojiGraphicsItem::nextFrame);
        m_frameTimer->start(m_frames.front().duration);
    }
}
