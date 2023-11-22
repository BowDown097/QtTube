#include "newpipeimportwizard.h"
#include "shared/choosesubspage.h"
#include "shared/choosewatchhistorypage.h"
#include "ui/forms/settings/data-wizards/entityselecttablemodel.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlQuery>

constexpr const char* dbSubtitle = "Select the newpipe.db file inside of the NewPipe data folder.";

constexpr const char* introInfo = "This wizard will help you import data from NewPipe into QtTube.\n"
                                  "Check the box(es) for the data you wish to import, then continue.\n"
                                  "<b>This data specifically comes from the \"Export database\" feature.<b>";

NewPipeImportWizard::NewPipeImportWizard(QWidget* parent)
    : DataWizard(Page_Conclusion, "NewPipe Import Wizard", parent)
{
    setPage(Page_Intro, new NewPipeImportIntroPage(this));
    setPage(Page_Db, new NewPipeImportDbPage(this));
    setStartId(Page_Intro);
}

NewPipeImportIntroPage::NewPipeImportIntroPage(QWidget* parent)
    : IntroPage(introInfo, "newpipe.import.watch_history", parent)
{
    registerField("newpipe.import.subs", subsCheckBox);
}

NewPipeImportDbPage::NewPipeImportDbPage(QWidget* parent)
    : ImportFileSelectPage("Database", dbSubtitle, "newpipe.db", 0, parent)
{
    connect(this, &ImportFileSelectPage::fileSelected, this, &NewPipeImportDbPage::verifyFile);
}

NewPipeImportDbPage::~NewPipeImportDbPage()
{
    QSqlDatabase::removeDatabase("NewPipe");
}

int NewPipeImportDbPage::nextId() const
{
    return field("newpipe.import.subs").toBool()
        ? NewPipeImportWizard::Page_ChooseSubs
        : NewPipeImportWizard::Page_ChooseWatchHistory;
}

void NewPipeImportDbPage::verifyFile(const QString& fileName)
{
    if (QSqlDatabase::contains("NewPipe"))
        QSqlDatabase::removeDatabase("NewPipe");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "NewPipe");
    db.setDatabaseName(fileName);

    if (!db.open())
    {
        pathEdit->setText("Invalid file selected");
        return;
    }

    if (field("newpipe.import.subs").toBool())
    {
        QSqlQuery subsQuery(QSqlDatabase::database("NewPipe"));
        if (!subsQuery.exec("SELECT url, name FROM subscriptions"))
        {
            pathEdit->setText("Invalid file selected");
            return;
        }

        QList<Entity> outSubs;
        while (subsQuery.next())
        {
            QString id = subsQuery.value(0).toString().remove("https://www.youtube.com/channel/");
            QString name = subsQuery.value(1).toString();
            outSubs.append(Entity(id, name));
        }

        wizard()->setPage(NewPipeImportWizard::Page_ChooseSubs, new ChooseSubsPage(
            outSubs,
            NewPipeImportWizard::Page_Conclusion,
            "newpipe.import.watch_history", NewPipeImportWizard::Page_ChooseWatchHistory,
            wizard()
        ));
    }

    if (field("newpipe.import.watch_history").toBool())
    {
        QSqlQuery videosQuery(QSqlDatabase::database("NewPipe"));
        if (!videosQuery.exec("SELECT url, title FROM streams"))
        {
            pathEdit->setText("Invalid file selected");
            return;
        }

        QList<Entity> outVideos;
        while (videosQuery.next())
        {
            QString id = videosQuery.value(0).toString().remove("https://www.youtube.com/watch?v=");
            QString name = QStringLiteral("<a href=\"%1\">%2</a>").arg(
                videosQuery.value(0).toString(),
                videosQuery.value(1).toString()
            );
            outVideos.append(Entity(id, name));
        }

        wizard()->setPage(NewPipeImportWizard::Page_ChooseWatchHistory, new ChooseWatchHistoryPage(
            outVideos, NewPipeImportWizard::Page_Conclusion, wizard()
        ));
    }

    pathEdit->setText(fileName);
    emit completeChanged();
}
