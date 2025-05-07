#pragma once
#include "innertube/objects/backstage/poll/pollchoice.h"
#include "ui/widgets/clickablewidget.h"
#include <QProgressBar>

class QHBoxLayout;
class QLabel;

class BackstagePollChoiceRenderer : public ClickableWidget<QProgressBar>
{
    Q_OBJECT
public:
    explicit BackstagePollChoiceRenderer(QWidget* parent = nullptr);
    bool hasStyle() const;
    void reset();
    void setData(const InnertubeObjects::PollChoice& pollChoice);
    void setValue(double value, bool selected);

    double voteRatioIfNotSelected() const { return m_data.voteRatioIfNotSelected; }
    double voteRatioIfSelected() const { return m_data.voteRatioIfSelected; }
private:
    QLabel* m_choiceTextLabel;
    InnertubeObjects::PollChoice m_data;
    QHBoxLayout* m_layout;
    QLabel* m_percentageLabel;
private slots:
    void choose();
};
