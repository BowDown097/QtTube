#pragma once
#include "qttube-plugin/components/pluginsettingswindow.h"

namespace Ui {
class SettingsForm;
}

class QAbstractButton;
class QButtonGroup;
class QCheckBox;

class SettingsForm : public QtTube::PluginSettingsWindow
{
    Q_OBJECT
public:
    explicit SettingsForm(QWidget* parent = nullptr);
    ~SettingsForm();

    bool savePending() const override;
    void saveSettings() override;
private:
    QButtonGroup* pluginActiveButtonGroup;
    Ui::SettingsForm* ui;

    void handleSponsorCategory(QStringList& categories, const QString& category, QCheckBox* checkBox);
private slots:
    void checkDownloadPath(const QString& text);
    void checkExternalPlayer(const QString& text);
    void clearCache();
    void currentChanged(int index);
    //void openExportWizard();
    void openImportWizard();
    void pluginActiveButtonToggled(QAbstractButton* button, bool checked);
    void selectDownloadPath();
    void selectExternalPlayer();
    void showChannelFilterTable();
    void showTermFilterTable();
    void toggleDeArrowSettings(bool checked);
    void toggleWebPlayerSettings(bool checked);
};
