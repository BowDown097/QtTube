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
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    Ui::SettingsForm* ui;
    void handleSponsorCategory(QStringList& categories, const QString& category, QCheckBox* checkBox);
private slots:
    void checkDownloadPath(const QString& text);
    void checkExternalPlayer(const QString& text);
    void clearCache();
    void enableSaveButton();
    //void openExportWizard();
    void openImportWizard();
    void saveSettings();
    void selectDownloadPath();
    void selectExternalPlayer();
    void showChannelFilterTable();
    void showTermFilterTable();
    void toggleDeArrowSettings(bool checked);
    void toggleWebPlayerSettings(bool checked);
};
