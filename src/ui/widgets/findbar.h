#ifndef FINDBAR_H
#define FINDBAR_H
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

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

#endif // FINDBAR_H
