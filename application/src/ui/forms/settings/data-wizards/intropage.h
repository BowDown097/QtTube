#pragma once
#include <QWizardPage>

class QCheckBox;
class QLabel;
class QVBoxLayout;

class IntroPage : public QWizardPage
{
public:
    IntroPage(const QString& infoText, const QString& watchHistoryField, QWidget* parent = nullptr);
    bool isComplete() const override;
protected:
    QCheckBox* subsCheckBox;
    QCheckBox* watchHistoryCheckBox;
private:
    QLabel* info;
    QVBoxLayout* layout;
};
