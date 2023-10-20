#ifndef TAKEOUTIMPORTWIZARD_H
#define TAKEOUTIMPORTWIZARD_H
#include "shared/importfileselectpage.h"
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QWizard>

class TakeoutImportWizard : public QWizard
{
public:
    enum { Page_Intro, Page_Subs, Page_ChooseSubs, Page_WatchHistory, Page_ChooseWatchHistory, Page_Conclusion };
    explicit TakeoutImportWizard(QWidget* parent = nullptr);
};

class TakeoutImportIntroPage : public QWizardPage
{
public:
    explicit TakeoutImportIntroPage(QWidget* parent = nullptr);
    bool isComplete() const override { return subsCheckBox->isChecked() || watchHistoryCheckBox->isChecked(); }
    int nextId() const override;
private:
    QLabel* info;
    QVBoxLayout* layout;
    QCheckBox* subsCheckBox;
    QCheckBox* watchHistoryCheckBox;
};

class TakeoutImportSubsPage : public ImportFileSelectPage
{
    Q_OBJECT
public:
    explicit TakeoutImportSubsPage(QWidget* parent = nullptr);
private slots:
    void verifyFile(const QString& fileName);
};

class TakeoutImportWatchHistoryPage : public ImportFileSelectPage
{
    Q_OBJECT
public:
    explicit TakeoutImportWatchHistoryPage(QWidget* parent = nullptr);
private slots:
    void verifyFile(const QString& fileName);
};

#endif // TAKEOUTIMPORTWIZARD_H
