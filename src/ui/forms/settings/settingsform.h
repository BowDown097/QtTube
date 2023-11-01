#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H
#include <QCheckBox>

namespace Ui {
class SettingsForm;
}

class SettingsForm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsForm(QWidget* parent = nullptr);
    ~SettingsForm();
private slots:
    //void openExportWizard();
    void openImportWizard();
    void saveSettings();
    void showChannelFilterTable();
    void showTermFilterTable();
    void toggleDeArrowSettings(bool checked);
private:
    Ui::SettingsForm* ui;
    void tryAddSponsorBlockCategory(QStringList& sponsorBlockCategories, const QString& category, QCheckBox* checkBox);
};

#endif // SETTINGSFORM_H
