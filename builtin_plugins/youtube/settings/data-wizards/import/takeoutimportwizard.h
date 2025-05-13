#pragma once
#include "settings/data-wizards/datawizard.h"
#include "settings/data-wizards/intropage.h"
#include "shared/importfileselectpage.h"

class TakeoutImportWizard : public DataWizard
{
public:
    enum { Page_Intro, Page_Subs, Page_ChooseSubs, Page_WatchHistory, Page_ChooseWatchHistory, Page_Conclusion };
    explicit TakeoutImportWizard(QWidget* parent = nullptr);
};

class TakeoutImportIntroPage : public IntroPage
{
public:
    explicit TakeoutImportIntroPage(QWidget* parent = nullptr);
    int nextId() const override;
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
