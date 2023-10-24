#include "intropage.h"

IntroPage::IntroPage(const QString& infoText, const QString& watchHistoryField, QWidget* parent)
    : QWizardPage(parent),
      info(new QLabel(infoText, this)),
      layout(new QVBoxLayout(this)),
      subsCheckBox(new QCheckBox("Subscriptions", this)),
      watchHistoryCheckBox(new QCheckBox("Watch History", this))
{
    setTitle("Introduction");

    info->setTextFormat(Qt::RichText);
    info->setWordWrap(true);

    registerField(watchHistoryField, watchHistoryCheckBox);

    layout->addWidget(info);
    layout->addWidget(subsCheckBox);
    layout->addWidget(watchHistoryCheckBox);

    connect(subsCheckBox, &QCheckBox::clicked, this, &IntroPage::completeChanged);
    connect(watchHistoryCheckBox, &QCheckBox::clicked, this, &IntroPage::completeChanged);
}
