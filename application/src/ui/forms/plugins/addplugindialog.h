#pragma once
#include "basepluginentry.h"
#include "plugins/pluginmanager.h"
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
    Ui::AddPluginDialog* ui;
private slots:
    void attemptAdd();
    void getOpenFile();
};

class AddPluginDialogEntry : public BasePluginEntry
{
    Q_OBJECT
public:
    explicit AddPluginDialogEntry(PluginData* data, QWidget* parent = nullptr);
    QRadioButton* activeButton() const { return m_activeButton; }
    PluginData* data() const { return m_data; }
private:
    QRadioButton* m_activeButton;
    PluginData* m_data;
};
