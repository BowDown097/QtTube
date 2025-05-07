#pragma once
#include "shared/importfileselectpage.h"
#include "ui/forms/settings/data-wizards/datawizard.h"
#include "ui/forms/settings/data-wizards/entityselecttablemodel.h"
#include "ui/forms/settings/data-wizards/intropage.h"

class QProgressDialog;

struct GrayjayImportWizard : DataWizard
{
    enum { Page_Intro, Page_Subs, Page_ChooseSubs, Page_WatchHistory, Page_ChooseWatchHistory, Page_Conclusion };
    explicit GrayjayImportWizard(QWidget* parent = nullptr);
};

struct GrayjayImportIntroPage : IntroPage
{
    explicit GrayjayImportIntroPage(QWidget* parent = nullptr);
    int nextId() const override;
};

class GrayjayImportSubsPage : public ImportFileSelectPage
{
    Q_OBJECT
public:
    explicit GrayjayImportSubsPage(QWidget* parent = nullptr);
private:
    QProgressDialog* progressDialog;
    QList<Entity> subs;
private slots:
    void tickProgress();
    void trySub(const QString& channelId);
    void verifyFile(const QString& fileName);
signals:
    void progress();
};

class GrayjayImportWatchHistoryPage : public ImportFileSelectPage
{
    Q_OBJECT
public:
    explicit GrayjayImportWatchHistoryPage(QWidget* parent = nullptr);
private slots:
    void verifyFile(const QString& fileName);
};
