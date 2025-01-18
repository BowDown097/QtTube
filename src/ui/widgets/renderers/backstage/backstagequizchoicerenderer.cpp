#include "backstagequizchoicerenderer.h"
#include "innertube.h"

constexpr QLatin1String BaseStylesheet("border: 1px solid");
constexpr QLatin1String CorrectAnswerStylesheet("border: 1px solid #2ba640");
constexpr QLatin1String WrongAnswerStylesheet("border: 1px solid #f57");

BackstageQuizChoiceRenderer::BackstageQuizChoiceRenderer(QWidget* parent)
    : ClickableWidget<QLabel>(parent)
{
    setContentsMargins(8, 8, 8, 8);
    setStyleSheet(BaseStylesheet);
    connect(this, &ClickableWidget<QLabel>::clicked, this, &BackstageQuizChoiceRenderer::choose);
}

void BackstageQuizChoiceRenderer::choose()
{
    m_isSelected = !m_isSelected;
    updateBorder();

    const QJsonValue endpoint = !m_isSelected ? m_data.selectServiceEndpoint : m_data.deselectServiceEndpoint;
    InnerTube::instance()->get<InnertubeEndpoints::PerformCommentAction>(
        endpoint["performCommentActionEndpoint"]["action"].toString());
}

void BackstageQuizChoiceRenderer::setData(const InnertubeObjects::QuizChoice& quizChoice)
{
    m_data = quizChoice;
    m_isSelected = quizChoice.isSelected;

    setText(quizChoice.text.text);
    updateBorder();
}

void BackstageQuizChoiceRenderer::updateBorder()
{
    if (m_isSelected)
        setStyleSheet(m_data.isCorrect ? CorrectAnswerStylesheet : WrongAnswerStylesheet);
    else
        setStyleSheet(BaseStylesheet);
}
