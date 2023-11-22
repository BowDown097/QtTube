#ifndef GRAYJAYIMPORTWIZARD_H
#define GRAYJAYIMPORTWIZARD_H
#include "shared/importfileselectpage.h"
#include "ui/forms/settings/data-wizards/datawizard.h"
#include "ui/forms/settings/data-wizards/entityselecttablemodel.h"
#include "ui/forms/settings/data-wizards/intropage.h"

class QProgressDialog;

constexpr const char* introInfo = "This wizard will help you import data from Grayjay into QtTube.\n"
                                  "Check the box(es) for the data you wish to import, then continue.";

class GrayjayImportWizard : public DataWizard
{
public:
    enum { Page_Intro, Page_Subs, Page_ChooseSubs, Page_WatchHistory, Page_ChooseWatchHistory, Page_Conclusion };
    explicit GrayjayImportWizard(QWidget* parent = nullptr);
};

class GrayjayImportIntroPage : public IntroPage
{
public:
    explicit GrayjayImportIntroPage(QWidget* parent = nullptr)
        : IntroPage(introInfo, "grayjay.import.watch_history", parent) {}
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

#endif // GRAYJAYIMPORTWIZARD_H
