#include "videothumbnailwidget.h"
#include "http.h"
#include <QProgressBar>

constexpr QLatin1String LengthStylesheet("background: rgba(0, 0, 0, 0.75); color: #fff; padding: 0 1px");
constexpr QLatin1String ProgressStylesheet(R"(
    QProgressBar { background-color: #717171; }
    QProgressBar::chunk { background-color: #f00; }
)");

VideoThumbnailWidget::VideoThumbnailWidget(QWidget* parent)
    : ClickableWidget<QLabel>(parent), m_lengthLabel(new QLabel(this)), m_progressBar(new QProgressBar(this))
{
    setClickable(true);
    setMinimumSize(1, 1);
    setScaledContents(true);

    m_lengthLabel->hide();
    m_lengthLabel->setFont(QFont(font().toString(), 9, QFont::Bold));
    m_lengthLabel->setStyleSheet(LengthStylesheet);

    m_progressBar->hide();
    m_progressBar->setFixedHeight(3);
    m_progressBar->setStyleSheet(ProgressStylesheet);
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
    setPixmap(pixmap);
    emit thumbnailSet();
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
