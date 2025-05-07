#pragma once
#include "innertube/objects/backstage/quiz/quizchoice.h"
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

class QLabel;

class BackstageQuizChoiceRenderer : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit BackstageQuizChoiceRenderer(QWidget* parent = nullptr);

    const InnertubeObjects::QuizChoice& data() const { return m_data; }
    bool isSelected() const { return m_isSelected; }

    void setData(const InnertubeObjects::QuizChoice& quizChoice);
private:
    InnertubeObjects::QuizChoice m_data;
    bool m_isSelected;

    void updateBorder();
private slots:
    void choose();
};
