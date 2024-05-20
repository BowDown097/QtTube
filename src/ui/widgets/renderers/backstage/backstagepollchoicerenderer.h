#pragma once
#include "innertube/objects/backstage/poll/pollchoice.h"
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QProgressBar;

class BackstagePollChoiceRenderer : public QWidget
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
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
private:
    QLabel* m_choiceTextLabel;
    InnertubeObjects::PollChoice m_data;
    QHBoxLayout* m_innerLayout;
    QLabel* m_percentageLabel;
    QProgressBar* m_progressBar;
signals:
    void clicked();
};
