#pragma once
#include <QWizardPage>

class QLabel;
class QVBoxLayout;

class BasicInfoPage : public QWizardPage
{
public:
    BasicInfoPage(const QString& title, const QString& info, QWidget* parent = nullptr);
private:
    QLabel* infoLabel;
    QVBoxLayout* layout;
};
