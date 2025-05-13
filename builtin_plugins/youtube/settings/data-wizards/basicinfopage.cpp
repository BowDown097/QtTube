#include "basicinfopage.h"
#include <QBoxLayout>
#include <QLabel>

BasicInfoPage::BasicInfoPage(const QString& title, const QString& info, QWidget* parent)
    : QWizardPage(parent), infoLabel(new QLabel(info, this)), layout(new QVBoxLayout(this))
{
    setTitle(title);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);
}
