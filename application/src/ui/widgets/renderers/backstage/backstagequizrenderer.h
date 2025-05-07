#pragma once
#include "innertube/objects/backstage/quiz/quiz.h"
#include <QWidget>

class BackstageQuizChoiceRenderer;
class QLabel;
class QVBoxLayout;

class BackstageQuizRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BackstageQuizRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Quiz& quiz);
private:
    QList<BackstageQuizChoiceRenderer*> choices;
    InnertubeObjects::Quiz data;
    QVBoxLayout* layout;
    QLabel* voteCount;
private slots:
    void quizChoiceClicked();
signals:
    void explanationUpdated();
};
