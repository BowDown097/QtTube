#include "backstagepollchoicerenderer.h"
#include "innertube.h"
#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>

constexpr const char* notSelectedStylesheet = "QProgressBar::chunk { background-color: rgba(255, 255, 255, 0.2) }";

BackstagePollChoiceRenderer::BackstagePollChoiceRenderer(QWidget* parent)
    : m_choiceTextLabel(new QLabel(this)),
      m_percentageLabel(new QLabel(this)),
      m_progressBar(new QProgressBar(this))
{
    m_choiceTextLabel->setMaximumWidth(width() - 100);

    m_progressBar->setFixedWidth(width());
    m_progressBar->setMaximum(100);
    m_progressBar->setTextVisible(false);

    m_innerLayout = new QHBoxLayout(m_progressBar);
    m_innerLayout->addWidget(m_choiceTextLabel, 0, Qt::AlignLeft);
    m_innerLayout->addWidget(m_percentageLabel, 0, Qt::AlignRight);
    m_innerLayout->setContentsMargins(5, 0, 20, 0);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void BackstagePollChoiceRenderer::enterEvent(QEnterEvent*)
#else
void BackstagePollChoiceRenderer::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::PointingHandCursor));
}

bool BackstagePollChoiceRenderer::hasStyle() const
{
    return !m_progressBar->styleSheet().isEmpty();
}

void BackstagePollChoiceRenderer::leaveEvent(QEvent*)
{
    setCursor(QCursor());
}

void BackstagePollChoiceRenderer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QJsonValue endpoint = hasStyle() || value() == -1 ? m_data.selectServiceEndpoint : m_data.deselectServiceEndpoint;
    emit clicked();
    InnerTube::instance()->get<InnertubeEndpoints::PerformCommentAction>(endpoint["performCommentActionEndpoint"]["action"].toString());
}

void BackstagePollChoiceRenderer::reset()
{
    m_progressBar->reset();
    m_progressBar->setStyleSheet(QString());
    m_percentageLabel->clear();
}

void BackstagePollChoiceRenderer::setData(const InnertubeObjects::PollChoice& pollChoice)
{
    m_data = pollChoice;
    m_choiceTextLabel->setText(pollChoice.text.text);
}

void BackstagePollChoiceRenderer::setValue(double value, bool selected)
{
    m_percentageLabel->setText(selected ? m_data.votePercentageIfSelected : m_data.votePercentageIfNotSelected);
    m_progressBar->setStyleSheet(selected ? QString() : notSelectedStylesheet);
    m_progressBar->setValue(value);
}

int BackstagePollChoiceRenderer::value() const
{
    return m_progressBar->value();
}
