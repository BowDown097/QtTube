#pragma once
#include "shared/importfileselectpage.h"
#include "ui/forms/settings/data-wizards/datawizard.h"
#include "ui/forms/settings/data-wizards/intropage.h"

class TakeoutImportWizard : public DataWizard
{
public:
    enum { Page_Intro, Page_Subs, Page_ChooseSubs, Page_WatchHistory, Page_ChooseWatchHistory, Page_Conclusion };
    explicit TakeoutImportWizard(QWidget* parent = nullptr);
};

class TakeoutImportIntroPage : public IntroPage
{
    static constexpr const char* introInfo = "This wizard will help you import YouTube data from Google Takeout into QtTube.\n"
                                             "Check the box(es) for the data you wish to import, then continue.\n"
                                             "To get this data if you don't have it already, go to "
                                             "<a href=\"https://takeout.google.com/takeout/custom/youtube\">this page</a>.\n"
                                             "<b>Make sure to change the format for History from HTML to JSON!</b>";
public:
    explicit TakeoutImportIntroPage(QWidget* parent = nullptr)
        : IntroPage(introInfo, "takeout.import.watch_history", parent) {}
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
