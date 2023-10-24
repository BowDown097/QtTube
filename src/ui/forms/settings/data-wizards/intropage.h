#ifndef INTROPAGE_H
#define INTROPAGE_H
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QWizardPage>

class IntroPage : public QWizardPage
{
public:
    IntroPage(const QString& infoText, const QString& watchHistoryField, QWidget* parent = nullptr);
    bool isComplete() const override { return subsCheckBox->isChecked() || watchHistoryCheckBox->isChecked(); }
protected:
    QCheckBox* subsCheckBox;
    QCheckBox* watchHistoryCheckBox;
private:
    QLabel* info;
    QVBoxLayout* layout;
};

#endif // INTROPAGE_H
