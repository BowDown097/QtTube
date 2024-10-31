#pragma once
#include <QWizard>

struct DataWizard : QWizard
{
    DataWizard(int conclusionId, const QString& title, QWidget* parent = nullptr);
};
