#include "settingswindow.h"
#include <QCheckBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

namespace QtTubePlugin
{
    void SettingsWindow::closeEvent(QCloseEvent* event)
    {
        if (savePending())
        {
            QMessageBox::StandardButton unsavedResponse = QMessageBox::warning(this,
                "Unsaved changes", "You have unsaved changes! Would you like to save them?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (unsavedResponse == QMessageBox::Yes)
                saveSettings();
        }

        QWidget::closeEvent(event);
    }

    void SettingsWindow::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Escape)
            close();
        else
            QWidget::keyPressEvent(event);
    }

    void SettingsWindow::setupSaveButton(QPushButton* saveButton, bool makeDynamic, QList<QWidget*> filteredWidgets)
    {
        connect(saveButton, &QPushButton::clicked, this, [this, saveButton] {
            saveSettings();
            saveButton->setEnabled(false);
        });

        if (!makeDynamic)
            return;

        if (!filteredWidgets.contains(saveButton))
            filteredWidgets.prepend(saveButton);

        auto bindEnabled = std::bind(&QWidget::setEnabled, saveButton, true);
        const QList<QWidget*> widgetChildren = findChildren<QWidget*>();

        for (QWidget* child : widgetChildren)
        {
            if (filteredWidgets.contains(child))
                continue;

            if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(child))
                connect(checkBox, &QCheckBox::clicked, bindEnabled);
            else if (QComboBox* comboBox = qobject_cast<QComboBox*>(child))
                connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), bindEnabled);
            else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(child))
                connect(lineEdit, &QLineEdit::textEdited, bindEnabled);
            else if (QPushButton* pushButton = qobject_cast<QPushButton*>(child))
                connect(pushButton, &QPushButton::clicked, bindEnabled);
            else if (QRadioButton* radioButton = qobject_cast<QRadioButton*>(child))
                connect(radioButton, &QRadioButton::clicked, bindEnabled);
            else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(child))
                connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), bindEnabled);
        }
    }
}
