#include "videothumbnailwidget.h"
#include "http.h"
#include <QApplication>
#include <QMouseEvent>

constexpr const char* progressStyle = R"(
    QProgressBar { background-color: #717171; }
    QProgressBar::chunk { background-color: #f00; }
)";

VideoThumbnailWidget::VideoThumbnailWidget(QWidget* parent)
    : QLabel(parent), lengthLabel(new QLabel(this)), progressBar(new QProgressBar(this))
{
    setMinimumSize(1, 1);
    setScaledContents(true);

    lengthLabel->setFont(QFont(qApp->font().toString(), 9, QFont::Bold));
    lengthLabel->setStyleSheet("background: rgba(0, 0, 0, 0.75); color: #fff; padding: 0 1px");
    lengthLabel->setVisible(false);

    progressBar->setFixedHeight(3);
    progressBar->setStyleSheet(progressStyle);
    progressBar->setVisible(false);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void VideoThumbnailWidget::enterEvent(QEnterEvent*)
#else
void VideoThumbnailWidget::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::PointingHandCursor));
}

void VideoThumbnailWidget::leaveEvent(QEvent*)
{
    setCursor(QCursor());
}

void VideoThumbnailWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
}

void VideoThumbnailWidget::resizeEvent(QResizeEvent* event)
{
    if (!lengthLabel->isVisible() && !lengthLabel->text().isEmpty())
        lengthLabel->setVisible(true);
    if (!progressBar->isVisible() && progressBar->value() > 0)
        progressBar->setVisible(true);

    lengthLabel->move(event->size().width() - lengthLabel->width() - 3, event->size().height() - lengthLabel->height() - 3);

    progressBar->move(0, event->size().height() - 3);
    progressBar->setFixedWidth(event->size().width());
}

void VideoThumbnailWidget::setData(const HttpReply& reply)
{
    if (reply.statusCode() != 200)
        return;

    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    setPixmap(pixmap.scaled(
        preferredSize.height() == 0 ? QSize(preferredSize.width(), height()) : preferredSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation)
    );

    emit thumbnailSet();
}

void VideoThumbnailWidget::setPreferredSize(const QSize& size)
{
    preferredSize = size;
    if (size.height() > 0)
        setFixedSize(size);
}

void VideoThumbnailWidget::setProgress(int progress, int length)
{
    progressBar->setMaximum(length);
    progressBar->setValue(progress);
}

void VideoThumbnailWidget::setUrl(const QString& url)
{
    HttpReply* reply = Http::instance().get(url);
    connect(reply, &HttpReply::finished, this, &VideoThumbnailWidget::setData);
}
