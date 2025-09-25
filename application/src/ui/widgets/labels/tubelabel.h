#pragma once
#include "ui/widgets/clickablewidget.h"
#include <QLabel>
#include <QRegularExpression>

class HttpReply;

class TubeLabel : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    enum ImageFlag
    {
        NoImageFlags = 0x0,
        Rounded = 0x1,
        Cached = 0x2,
        KeepAspectRatio = 0x4
    };
    Q_DECLARE_FLAGS(ImageFlags, ImageFlag)

    explicit TubeLabel(QWidget* parent = nullptr);
    explicit TubeLabel(const QString& text, QWidget* parent = nullptr);

    void setElideMode(Qt::TextElideMode mode) { m_elideMode = mode; }
    void setImage(const QUrl& url, ImageFlags flags = ImageFlag::NoImageFlags);
    void setMaximumLines(int lines);
    void setPixmap(const QPixmap& pixmap);
    void setText(const QString& text, bool processRemoteImages = false, ImageFlags remoteImageFlags = ImageFlag::NoImageFlags);

    QRect alignedRect(QRect rect) const;
    QRect boundingRect() const;
    QRect boundingRectOfLineAt(const QPoint& point) const;
    Qt::TextElideMode elideMode() const { return m_elideMode; }
    int heightForWidth(int w) const override;
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private:
    int m_calculatedMaximumHeight = -1;
    Qt::TextElideMode m_elideMode = Qt::ElideNone;
    ImageFlags m_imageFlags = ImageFlag::NoImageFlags;
    int m_imagePixmapHeight{};
    int m_imagePixmapWidth{};
    bool m_isImage{};
    QList<QRect> m_lineRects;
    int m_maximumLines = -1;
    QString m_rawText;
    QList<std::pair<QRegularExpressionMatch, QString>> m_remoteImageDataMap;
    std::unordered_map<const HttpReply*, QRegularExpressionMatch> m_remoteImageReplyMap;

    void calculateAndSetLineRects();
    std::unique_ptr<QTextDocument> createTextDocument(const QString& text, int textWidth) const;
    void processRemoteImages(QString text, ImageFlags flags);
    int textLineWidth() const;
    void updateMarginsForImageAspectRatio();
private slots:
    void remoteImageDownloaded(QString text, const HttpReply& reply);
    void setImageData(const HttpReply& reply);
signals:
    void imageSet();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TubeLabel::ImageFlags)
