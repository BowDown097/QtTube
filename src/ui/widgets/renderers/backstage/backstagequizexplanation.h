#pragma once
#include <QWidget>

namespace InnertubeObjects { struct Quiz; }

class QVBoxLayout;
class TubeLabel;

class BackstageQuizExplanation : public QWidget
{
    Q_OBJECT
public:
    explicit BackstageQuizExplanation(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Quiz& quiz, const QString& explanation);
private:
    TubeLabel* m_content;
    QString m_explanation;
    TubeLabel* m_header;
    QVBoxLayout* m_layout;
    TubeLabel* m_readMoreLabel;
    QString m_readMoreText;
    QString m_showLessText;
private slots:
    void toggleReadMore();
signals:
    void readMoreClicked();
};
