#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H
#include <QWidget>

namespace Ui {
class SettingsForm;
}

class SettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsForm(QWidget *parent = nullptr);
    ~SettingsForm();
private slots:
    void saveSettings();
    void toggleDeArrowSettings(bool checked);
private:
    Ui::SettingsForm *ui;
};

#endif // SETTINGSFORM_H
