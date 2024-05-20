#pragma once
#include <QWizard>

class DataWizard : public QWizard
{
public:
    DataWizard(int conclusionId, const QString& title, QWidget* parent = nullptr);
};
