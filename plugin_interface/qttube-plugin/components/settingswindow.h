#pragma once
#include <QWidget>

class QPushButton;

namespace QtTubePlugin
{
    class SettingsWindow : public QWidget
    {
        Q_OBJECT
    public:
        explicit SettingsWindow(QWidget* parent = nullptr) : QWidget(parent) {}
        virtual ~SettingsWindow() = default;
        virtual bool savePending() const = 0;
        virtual void saveSettings() = 0;
    protected:
        void closeEvent(QCloseEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void setupSaveButton(QPushButton* saveButton, bool makeDynamic, QList<QWidget*> filteredWidgets = {});
    };
}
