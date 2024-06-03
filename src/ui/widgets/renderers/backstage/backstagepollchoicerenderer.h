#pragma once
#include "innertube/objects/backstage/poll/pollchoice.h"
#include "ui/widgets/clickablewidget.h"

class QHBoxLayout;
class QLabel;
class QProgressBar;

class BackstagePollChoiceRenderer : public ClickableWidget<QWidget>
{
    Q_OBJECT
public:
    explicit BackstagePollChoiceRenderer(QWidget* parent = nullptr);
    bool hasStyle() const;
    void reset();
    void setData(const InnertubeObjects::PollChoice& pollChoice);

    void setValue(double value, bool selected);
    int value() const;

    double voteRatioIfNotSelected() const { return m_data.voteRatioIfNotSelected; }
    double voteRatioIfSelected() const { return m_data.voteRatioIfSelected; }
private:
    QLabel* m_choiceTextLabel;
    InnertubeObjects::PollChoice m_data;
    QHBoxLayout* m_innerLayout;
    QLabel* m_percentageLabel;
    QProgressBar* m_progressBar;
private slots:
    void choose();
};
