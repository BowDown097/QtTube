#pragma once
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
    void handleSponsorCategory(QStringList& categories, const QString& category, QCheckBox* checkBox);
private slots:
    void checkExternalPlayer(const QString& text);
    void clearCache();
    //void openExportWizard();
    void openImportWizard();
    void saveSettings();
    void selectExternalPlayer();
    void showChannelFilterTable();
    void showTermFilterTable();
    void toggleDeArrowSettings(bool checked);
    void toggleWebPlayerSettings(bool checked);
};
