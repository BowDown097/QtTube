#include "backstagepollchoicerenderer.h"
#include "innertube.h"
#include <QBoxLayout>
#include <QLabel>

constexpr QLatin1String NotSelectedStylesheet("QProgressBar::chunk { background-color: rgba(255, 255, 255, 0.2) }");

BackstagePollChoiceRenderer::BackstagePollChoiceRenderer(QWidget* parent)
    : ClickableWidget<QProgressBar>(parent),
      m_choiceTextLabel(new QLabel(this)),
      m_layout(new QHBoxLayout(this)),
      m_percentageLabel(new QLabel(this))
{
    setClickable(true);
    setMaximum(100);
    setTextVisible(false);

    m_layout->addWidget(m_choiceTextLabel, 0, Qt::AlignLeft);
    m_layout->addWidget(m_percentageLabel, 0, Qt::AlignRight);
    m_layout->setContentsMargins(5, 0, 20, 0);

    connect(this, &ClickableWidget<QProgressBar>::clicked, this, &BackstagePollChoiceRenderer::choose);
}

void BackstagePollChoiceRenderer::choose()
{
    const QJsonValue endpoint = hasStyle() || value() == -1 ? m_data.selectServiceEndpoint : m_data.deselectServiceEndpoint;
    InnerTube::instance()->get<InnertubeEndpoints::PerformCommentAction>(
        endpoint["performCommentActionEndpoint"]["action"].toString());
}

bool BackstagePollChoiceRenderer::hasStyle() const
{
    return !styleSheet().isEmpty();
}

void BackstagePollChoiceRenderer::reset()
{
    QProgressBar::reset();
    setStyleSheet(QString());
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
    setStyleSheet(selected ? QLatin1String() : NotSelectedStylesheet);
    QProgressBar::setValue(value);
}
