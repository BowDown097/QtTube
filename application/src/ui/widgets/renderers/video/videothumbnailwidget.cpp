#include "videothumbnailwidget.h"
#include <QProgressBar>

constexpr QLatin1String LengthStylesheet("background: rgba(0, 0, 0, 0.75); color: #fff; padding: 0 1px");
constexpr QLatin1String ProgressStylesheet(R"(
    QProgressBar { background-color: #717171; }
    QProgressBar::chunk { background-color: #f00; }
)");

VideoThumbnailWidget::VideoThumbnailWidget(QWidget* parent)
    : TubeLabel(parent),
      m_lengthLabel(new QLabel(this)),
      m_progressBar(new QProgressBar(this)),
      m_sourceIconLabel(new TubeLabel(this))
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

    m_sourceIconLabel->hide();
    m_sourceIconLabel->setFixedSize(fontMetrics().height(), fontMetrics().height());
    m_sourceIconLabel->setScaledContents(true);
}

void VideoThumbnailWidget::resizeEvent(QResizeEvent* event)
{
    int sourceIconWidth{};
    if (m_hasSourceIcon)
    {
        m_sourceIconLabel->show();
        sourceIconWidth = m_sourceIconLabel->width();
    }

    m_lengthLabel->setVisible(!m_lengthLabel->text().isEmpty());
    m_progressBar->setVisible(m_progressBar->value() > 0);

    m_sourceIconLabel->move(
        event->size().width() - sourceIconWidth - 3,
        event->size().height() - m_sourceIconLabel->height() - 3);
    m_lengthLabel->move(
        event->size().width() - sourceIconWidth - 3 - m_lengthLabel->width() - 3,
        event->size().height() - m_lengthLabel->height() - 3);
    m_progressBar->move(0, event->size().height() - 3);

    m_progressBar->setFixedWidth(event->size().width());
}

void VideoThumbnailWidget::setProgress(int progress, int length)
{
    m_progressBar->setMaximum(length);
    m_progressBar->setValue(progress);
}

void VideoThumbnailWidget::setSourceIconUrl(const char* sourceIconUrl)
{
    if (sourceIconUrl)
    {
        m_hasSourceIcon = true;
        m_sourceIconLabel->setImage(QUrl(sourceIconUrl), TubeLabel::Cached | TubeLabel::KeepAspectRatio);
    }
}
