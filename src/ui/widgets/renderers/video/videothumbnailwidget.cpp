#include "videothumbnailwidget.h"
#include "http.h"
#include <QApplication>
#include <QProgressBar>

constexpr const char* progressStyle = R"(
    QProgressBar { background-color: #717171; }
    QProgressBar::chunk { background-color: #f00; }
)";

VideoThumbnailWidget::VideoThumbnailWidget(QWidget* parent)
    : ClickableWidget<QLabel>(true, false, parent), m_lengthLabel(new QLabel(this)), m_progressBar(new QProgressBar(this))
{
    setMinimumSize(1, 1);
    setScaledContents(true);

    m_lengthLabel->hide();
    m_lengthLabel->setFont(QFont(qApp->font().toString(), 9, QFont::Bold));
    m_lengthLabel->setStyleSheet("background: rgba(0, 0, 0, 0.75); color: #fff; padding: 0 1px");

    m_progressBar->hide();
    m_progressBar->setFixedHeight(3);
    m_progressBar->setStyleSheet(progressStyle);
}

void VideoThumbnailWidget::resizeEvent(QResizeEvent* event)
{
    m_lengthLabel->setVisible(!m_lengthLabel->text().isEmpty());
    m_progressBar->setVisible(m_progressBar->value() > 0);

    m_lengthLabel->move(event->size().width() - m_lengthLabel->width() - 3, event->size().height() - m_lengthLabel->height() - 3);
    m_progressBar->move(0, event->size().height() - 3);

    m_progressBar->setFixedWidth(event->size().width());
}

void VideoThumbnailWidget::setData(const HttpReply& reply)
{
    if (reply.statusCode() != 200)
        return;

    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    setPixmap(pixmap.scaled(
        m_preferredSize.height() == 0 ? QSize(m_preferredSize.width(), height()) : m_preferredSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation)
    );

    emit thumbnailSet();
}

void VideoThumbnailWidget::setPreferredSize(const QSize& size)
{
    m_preferredSize = size;
    if (size.height() > 0)
        setFixedSize(size);
}

void VideoThumbnailWidget::setProgress(int progress, int length)
{
    m_progressBar->setMaximum(length);
    m_progressBar->setValue(progress);
}

void VideoThumbnailWidget::setUrl(const QString& url)
{
    HttpReply* reply = Http::instance().get(url);
    connect(reply, &HttpReply::finished, this, &VideoThumbnailWidget::setData);
}
