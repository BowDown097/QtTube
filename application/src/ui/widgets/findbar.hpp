#pragma once
#include <QPointer>
#include <QWidget>

class CloseButton;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

class FindBar : public QWidget
{
public:
    explicit FindBar(QWidget* parent);
    void setReveal(bool reveal);
private:
    CloseButton* m_closeButton;
    int m_currentIndex{};
    QHBoxLayout* m_layout;
    QList<QPointer<QLabel>> m_matches;
    QLabel* m_matchesLabel;
    QPushButton* m_nextButton;
    QPushButton* m_previousButton;
    QLineEdit* m_searchBox;

    void clearMatches();
    void highlightMatch(const QPointer<QLabel>& label);
    void unhighlightMatch(const QPointer<QLabel>& label);
private slots:
    void goToNext();
    void goToPrevious();
    void initializeSearch(const QString& searchText);
    void jumpToCurrentMatch();
    void returnPressed();
};
