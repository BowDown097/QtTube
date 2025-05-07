#pragma once
#include "shared/importfileselectpage.h"
#include "ui/forms/settings/data-wizards/datawizard.h"
#include "ui/forms/settings/data-wizards/intropage.h"

class NewPipeImportWizard : public DataWizard
{
public:
    enum { Page_Intro, Page_Db, Page_ChooseSubs, Page_ChooseWatchHistory, Page_Conclusion };
    explicit NewPipeImportWizard(QWidget* parent = nullptr);
};

class NewPipeImportIntroPage : public IntroPage
{
public:
    explicit NewPipeImportIntroPage(QWidget* parent = nullptr);
    int nextId() const override { return NewPipeImportWizard::Page_Db; }
};

class NewPipeImportDbPage : public ImportFileSelectPage
{
    Q_OBJECT
public:
    explicit NewPipeImportDbPage(QWidget* parent = nullptr);
    ~NewPipeImportDbPage();
    int nextId() const override;
private slots:
    void verifyFile(const QString& fileName);
};
