#pragma once
#include <QWidget>

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
    int currentIndex{};
    QHBoxLayout* hbox;
    QList<QLabel*> matches;
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
