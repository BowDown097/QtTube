#pragma once
#include "basepluginentry.h"
#include "plugins/pluginentry.h"
#include <QDialog>
#include <QRadioButton>

namespace Ui {
class AddPluginDialog;
}

class QHBoxLayout;
class QVBoxLayout;

class AddPluginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddPluginDialog(QWidget* parent = nullptr);
    ~AddPluginDialog();
private:
    struct PluginSource
    {
        QFileInfo fileInfo;
        QString targetFileName;
    };

    Ui::AddPluginDialog* ui;

    PluginSource resolvePluginSource(const QString& input);
private slots:
    void attemptAdd();
    void getOpenFile();
};

class AddPluginDialogEntry : public BasePluginEntry
{
    Q_OBJECT
public:
    explicit AddPluginDialogEntry(PluginEntry* plugin, QWidget* parent = nullptr);
    QRadioButton* activeButton() const { return m_activeButton; }
    PluginEntry* plugin() const { return m_plugin; }
private:
    QRadioButton* m_activeButton;
    PluginEntry* m_plugin;
};
