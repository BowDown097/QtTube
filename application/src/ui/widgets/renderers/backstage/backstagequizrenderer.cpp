#include "backstagequizrenderer.h"
#include "backstagequizchoicerenderer.h"
#include "backstagequizexplanation.h"
#include <QBoxLayout>
#include <QLabel>

BackstageQuizRenderer::BackstageQuizRenderer(QWidget* parent)
    : layout(new QVBoxLayout(this)),
      voteCount(new QLabel(this))
{
    voteCount->setFont(QFont(font().toString(), font().pointSize() - 1));
    layout->addWidget(voteCount);
}

void BackstageQuizRenderer::quizChoiceClicked()
{
    BackstageQuizChoiceRenderer* senderChoice = qobject_cast<BackstageQuizChoiceRenderer*>(sender());
    senderChoice->setClickable(false);

    if (data.disableChangingQuizAnswer)
        for (BackstageQuizChoiceRenderer* choice : choices)
            choice->setClickable(false);

    if (BackstageQuizExplanation* explanation = findChild<BackstageQuizExplanation*>())
    {
        explanation->setData(data, senderChoice->data().explanation.text);
    }
    else if (const QString& explanationText = senderChoice->data().explanation.text; !explanationText.isEmpty())
    {
        explanation = new BackstageQuizExplanation(this);
        explanation->setData(data, explanationText);
        layout->addWidget(explanation);

        connect(explanation, &BackstageQuizExplanation::readMoreClicked, this, &BackstageQuizRenderer::explanationUpdated);
    }

    emit explanationUpdated();
}

void BackstageQuizRenderer::setData(const InnertubeObjects::Quiz& quiz)
{
    data = quiz;
    voteCount->setText(data.totalVotes.text);

    auto selectedChoice = std::ranges::find(data.choices, true, &InnertubeObjects::QuizChoice::isSelected);
    bool hasSelected = selectedChoice != data.choices.end();

    for (const InnertubeObjects::QuizChoice& quizChoice : data.choices)
    {
        BackstageQuizChoiceRenderer* quizChoiceRenderer = new BackstageQuizChoiceRenderer(this);
        quizChoiceRenderer->setData(quizChoice);
        quizChoiceRenderer->setFixedHeight(36);
        choices.append(quizChoiceRenderer);

        quizChoiceRenderer->setClickable(
            (data.disableChangingQuizAnswer && !quizChoice.isSelected && !hasSelected) ||
            (!data.disableChangingQuizAnswer && !quizChoice.isSelected));

        layout->addWidget(quizChoiceRenderer);
        connect(quizChoiceRenderer, &BackstageQuizChoiceRenderer::clicked, this, &BackstageQuizRenderer::quizChoiceClicked);
    }

    if (hasSelected && !selectedChoice->explanation.text.isEmpty())
    {
        BackstageQuizExplanation* explanation = new BackstageQuizExplanation(this);
        explanation->setData(quiz, selectedChoice->explanation.text);
        layout->addWidget(explanation);

        connect(explanation, &BackstageQuizExplanation::readMoreClicked, this, &BackstageQuizRenderer::explanationUpdated);
    }
}
