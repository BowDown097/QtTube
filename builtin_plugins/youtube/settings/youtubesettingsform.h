#pragma once
#include "qttube-plugin/components/settingswindow.h"

namespace Ui {
class YouTubeSettingsForm;
}

class QCheckBox;

class YouTubeSettingsForm : public QtTubePlugin::SettingsWindow
{
    Q_OBJECT
public:
    explicit YouTubeSettingsForm(QWidget* parent = nullptr);
    ~YouTubeSettingsForm() override;

    virtual bool savePending() const override;
    virtual void saveSettings() override;
private:
    Ui::YouTubeSettingsForm* ui;
    void handleSponsorCategory(QStringList& categories, const QString& category, QCheckBox* checkBox);
private slots:
    //void openExportWizard();
    void openImportWizard();
    void showChannelFilterTable();
};
