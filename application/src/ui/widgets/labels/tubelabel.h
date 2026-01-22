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
        KeepAspectRatio = 0x4,
        NoOptimizeForFixedSize = 0x8,
        LazyLoaded = 0x10
    };
    Q_DECLARE_FLAGS(ImageFlags, ImageFlag)

    explicit TubeLabel(QWidget* parent = nullptr);
    explicit TubeLabel(const QString& text, QWidget* parent = nullptr);

    void setElideMode(Qt::TextElideMode mode) { m_elideMode = mode; }
    void setImage(const QUrl& url, ImageFlags flags = NoImageFlags);
    void setMaximumLines(int lines);
    void setPixmap(const QPixmap& pixmap);
    void setScaledContents(bool enable);
    void setText(const QString& text, bool processRemoteImages = false, ImageFlags remoteImageFlags = NoImageFlags);

    QRect alignedRect(QRect rect) const;
    QRect boundingRect() const;
    QRect boundingRectOfLineAt(const QPoint& point) const;
    Qt::TextElideMode elideMode() const { return m_elideMode; }
    bool hasScaledContents() const { return m_scaledContents; }
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
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private:
    struct ImageData
    {
        QUrl lazyUrl;
        QSize size;
        ImageFlags flags = NoImageFlags;
        bool processing{};
    };

    std::optional<ImageData> m_imageData;
    QString m_rawText;
    QList<QRect> m_lineRects;
    int m_calculatedMaximumHeight = -1;
    int m_maximumLines = -1;
    Qt::TextElideMode m_elideMode = Qt::ElideNone;
    bool m_scaledContents{};

    void calculateAndSetLineRects();
    std::unique_ptr<QTextDocument> createTextDocument(const QString& text, int textWidth) const;
    bool isCachingImages() const;
    bool isLazyLoadingImages() const;
    int textLineWidth() const;
    void updateMarginsForImageAspectRatio();
private slots:
    void setImageData(const HttpReply& reply);
signals:
    void imageSet();
};

class RemoteImageProcessor : public QObject
{
    Q_OBJECT
public:
    RemoteImageProcessor(QString text, TubeLabel::ImageFlags flags, QObject* parent = nullptr);
private:
    QList<std::pair<QRegularExpressionMatch, QString>> m_dataMap;
    size_t m_pending;
private slots:
    void downloaded(QString text, QRegularExpressionMatch match, const HttpReply& reply);
signals:
    void textReady(const QString& text);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TubeLabel::ImageFlags)
