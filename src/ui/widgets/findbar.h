#pragma once
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

class FindBar : public QWidget
{
public:
    int currentIndex;
    QList<QLabel*> matches;

    explicit FindBar(QWidget* parent);
    void setReveal(bool reveal);
private:
    QHBoxLayout* hbox;
    QLabel* matchesLabel;
    QPushButton* nextButton;
    QPushButton* previousButton;
    QLineEdit* searchBox;
    void clearLabels();
private slots:
    void goToNext();
    void goToPrevious();
    void jumpToLabel();
    void initializeSearch(const QString& searchText);
};
