#pragma once
#include "qttube-plugin/objects/emoji.h"
#include <QGraphicsItem>

class HttpReply;

class EmojiGraphicsItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit EmojiGraphicsItem(const QtTube::Emoji& data, QGraphicsItem* parent = nullptr);
    const QtTube::Emoji& data() const { return m_data; }
    void setSize(int w, int h) { m_size = QSize(w, h); }
    void setSize(QSize size) { m_size = size; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
private:
    struct Frame
    {
        int duration;
        QPixmap pixmap;
    };

    qsizetype m_currentFrame{};
    const QtTube::Emoji& m_data;
    QList<Frame> m_frames;
    QTimer* m_frameTimer;
    QSize m_size;
private slots:
    void nextFrame();
    void setImageData(const HttpReply& reply);
    void updateWithData();
signals:
    void clicked();
};
