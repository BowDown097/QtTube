#ifndef IMPORTFILESELECTPAGE_H
#define IMPORTFILESELECTPAGE_H
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWizardPage>

class ImportFileSelectPage : public QWizardPage
{
    Q_OBJECT
public:
    ImportFileSelectPage(const QString& title, const QString& subtitle, const QString& targetFile,
                         int nextPage, QWidget* parent = nullptr);
    bool isComplete() const override;
    int nextId() const override { return nextPage; }
protected:
    QPushButton* browseButton;
    QHBoxLayout* browseLayout;
    QVBoxLayout* layout;
    int nextPage;
    QLineEdit* pathEdit;
    QString targetFile;
private:
    void browseForFile();
signals:
    void fileSelected(const QString&);
};

#endif // IMPORTFILESELECTPAGE_H
