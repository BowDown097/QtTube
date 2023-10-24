#ifndef DATAWIZARD_H
#define DATAWIZARD_H
#include <QWizard>

class DataWizard : public QWizard
{
public:
    DataWizard(int conclusionId, const QString& title, QWidget* parent = nullptr);
};

#endif // DATAWIZARD_H
