#pragma once
#include "shared/importfileselectpage.h"
#include "ui/forms/settings/data-wizards/datawizard.h"
#include "ui/forms/settings/data-wizards/entityselecttablemodel.h"
#include "ui/forms/settings/data-wizards/intropage.h"

class QProgressDialog;

class PipedImportWizard : public DataWizard
{
public:
    enum { Page_Intro, Page_Subs, Page_ChooseSubs, Page_WatchHistory, Page_ChooseWatchHistory, Page_Conclusion };
    explicit PipedImportWizard(QWidget* parent = nullptr);
};

class PipedImportIntroPage : public IntroPage
{
public:
    explicit PipedImportIntroPage(QWidget* parent = nullptr);
    int nextId() const override;
};

class PipedImportSubsPage : public ImportFileSelectPage
{
    Q_OBJECT
public:
    explicit PipedImportSubsPage(QWidget* parent = nullptr);
private slots:
    void verifyFile(const QString& fileName);
};

class PipedImportWatchHistoryPage : public ImportFileSelectPage
{
    Q_OBJECT
public:
    explicit PipedImportWatchHistoryPage(QWidget* parent = nullptr);
private:
    QProgressDialog* progressDialog;
    QList<Entity> videos;
private slots:
    void tickProgress();
    void verifyFile(const QString& fileName);
signals:
    void progress();
};
