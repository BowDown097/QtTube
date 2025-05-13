#pragma once
#include "qttube-plugin/components/pluginsettingswindow.h"

namespace Ui {
class YouTubeSettingsForm;
}

class QCheckBox;
class YouTubeSettings;

class YouTubeSettingsForm : public QtTube::PluginSettingsWindow
{
    Q_OBJECT
public:
    explicit YouTubeSettingsForm(YouTubeSettings* settings, QWidget* parent = nullptr);
    ~YouTubeSettingsForm();

    virtual bool savePending() const override;
    virtual void saveSettings() override;
private:
    YouTubeSettings* settings;
    Ui::YouTubeSettingsForm* ui;
    void handleSponsorCategory(QStringList& categories, const QString& category, QCheckBox* checkBox);
private slots:
    //void openExportWizard();
    void openImportWizard();
    void showChannelFilterTable();
    void toggleDeArrowSettings(bool checked);
};
