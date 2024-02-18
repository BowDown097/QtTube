#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H
#include <QWidget>

namespace Ui {
class SettingsForm;
}

class QCheckBox;

class SettingsForm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsForm(QWidget* parent = nullptr);
    ~SettingsForm();
private:
    Ui::SettingsForm* ui;
    void tryAddSponsorBlockCategory(QStringList& sponsorBlockCategories, const QString& category, QCheckBox* checkBox);
private slots:
    void clearCache();
    //void openExportWizard();
    void openImportWizard();
    void saveSettings();
    void showChannelFilterTable();
    void showTermFilterTable();
    void toggleDeArrowSettings(bool checked);
};

#endif // SETTINGSFORM_H
