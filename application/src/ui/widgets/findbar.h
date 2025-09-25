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
    CloseButton* closeButton;
    int currentIndex{};
    QHBoxLayout* hbox;
    QList<QPointer<QLabel>> matches;
    QLabel* matchesLabel;
    QPushButton* nextButton;
    QPushButton* previousButton;
    QLineEdit* searchBox;

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
