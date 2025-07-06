#pragma once
#include "qttube-plugin/components/settingswindow.h"

namespace Ui {
class SettingsForm;
}

class QAbstractButton;
class QButtonGroup;
class QCheckBox;

class SettingsForm : public QtTubePlugin::SettingsWindow
{
    Q_OBJECT
public:
    explicit SettingsForm(QWidget* parent = nullptr);
    ~SettingsForm() override;

    bool savePending() const override;
    void saveSettings() override;
private:
    QButtonGroup* pluginActiveButtonGroup;
    Ui::SettingsForm* ui;

    static QString extractPath(const QString& str);
private slots:
    void checkExternalPlayer(const QString& text);
    void clearCache();
    void currentChanged(int index);
    void pluginActiveButtonToggled(QAbstractButton* button, bool checked);
    void selectExternalPlayer();
    void showTermFilterTable();
};
