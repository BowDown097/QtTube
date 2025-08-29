#pragma once
#include <QDialog>

namespace Ui {
class AddPluginDialog;
}

class AddPluginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddPluginDialog(QWidget* parent = nullptr);
    ~AddPluginDialog();
private:
    Ui::AddPluginDialog* ui;
    QByteArray blockingRequest(const QUrl& url);
private slots:
    void attemptAdd();
    void getOpenFile();
};
