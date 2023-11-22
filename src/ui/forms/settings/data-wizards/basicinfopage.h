#ifndef BASICINFOPAGE_H
#define BASICINFOPAGE_H
#include <QWizardPage>

class QLabel;
class QVBoxLayout;

class BasicInfoPage : public QWizardPage
{
public:
    explicit BasicInfoPage(const QString& title, const QString& info, QWidget* parent = nullptr);
private:
    QLabel* infoLabel;
    QVBoxLayout* layout;
};

#endif // BASICINFOPAGE_H
