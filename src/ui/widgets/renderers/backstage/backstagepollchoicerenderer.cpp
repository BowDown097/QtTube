#include "backstagepollchoicerenderer.h"
#include "innertube.h"
#include <QBoxLayout>
#include <QLabel>
#include <QProgressBar>

constexpr const char* notSelectedStylesheet = "QProgressBar::chunk { background-color: rgba(255, 255, 255, 0.2) }";

BackstagePollChoiceRenderer::BackstagePollChoiceRenderer(QWidget* parent)
    : m_choiceTextLabel(new QLabel(this)),
      m_percentageLabel(new QLabel(this)),
      m_progressBar(new QProgressBar(this))
{
    setClickable(true, false);
    m_choiceTextLabel->setMaximumWidth(width() - 100);

    m_progressBar->setFixedWidth(width());
    m_progressBar->setMaximum(100);
    m_progressBar->setTextVisible(false);

    m_innerLayout = new QHBoxLayout(m_progressBar);
    m_innerLayout->addWidget(m_choiceTextLabel, 0, Qt::AlignLeft);
    m_innerLayout->addWidget(m_percentageLabel, 0, Qt::AlignRight);
    m_innerLayout->setContentsMargins(5, 0, 20, 0);

    connect(this, &ClickableWidget<QWidget>::clicked, this, &BackstagePollChoiceRenderer::choose);
}

void BackstagePollChoiceRenderer::choose()
{
    const QJsonValue endpoint = hasStyle() || value() == -1 ? m_data.selectServiceEndpoint : m_data.deselectServiceEndpoint;
    InnerTube::instance()->get<InnertubeEndpoints::PerformCommentAction>(endpoint["performCommentActionEndpoint"]["action"].toString());
}

bool BackstagePollChoiceRenderer::hasStyle() const
{
    return !m_progressBar->styleSheet().isEmpty();
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
