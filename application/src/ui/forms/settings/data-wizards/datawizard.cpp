#include "datawizard.h"
#include "conclusionpage.h"

DataWizard::DataWizard(int conclusionId, const QString& title, QWidget* parent) : QWizard(parent)
{
    setPage(conclusionId, new ConclusionPage(this));

#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif

    setOption(NoCancelButton);
    setWindowTitle(title);
}
