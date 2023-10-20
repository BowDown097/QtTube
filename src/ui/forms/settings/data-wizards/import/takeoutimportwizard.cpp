#include "takeoutimportwizard.h"
#include "shared/choosesubspage.h"
#include "shared/choosewatchhistorypage.h"
#include "ui/forms/settings/data-wizards/conclusionpage.h"
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>

constexpr const char* introInfo = "This wizard will help you import YouTube data from Google Takeout into QtTube.\n"
                                  "Check the box for the data you wish to import, then continue.\n"
                                  "To get this data if you don't have it already, go to "
                                  "<a href=\"https://takeout.google.com/takeout/custom/youtube\">this page</a>.\n"
                                  "<b>Make sure to change the format for History from HTML to JSON!</b>";

constexpr const char* subsSubtitle = "Select the subscriptions.csv file inside of the takeout folder.\n"
                                     "It should be inside \"Takeout/YouTube and YouTube Music/subscriptions\".";

constexpr const char* watchHistorySubtitle = "Select the watch-history.json file inside of the takeout folder.\n"
                                             "It should be inside \"Takeout/YouTube and YouTube Music/history\".";

bool readCSVRow(QTextStream& in, QStringList& row)
{
    static const int delta[][5] = {
        //  ,    "   \n    ?  eof
        {   1,   2,  -1,   0,  -1  }, // 0: parsing (store char)
        {   1,   2,  -1,   0,  -1  }, // 1: parsing (store column)
        {   3,   4,   3,   3,  -2  }, // 2: quote entered (no-op)
        {   3,   4,   3,   3,  -2  }, // 3: parsing inside quotes (store char)
        {   1,   3,  -1,   0,  -1  }, // 4: quote exited (no-op)
        // -1: end of row, store column, success
        // -2: eof inside quotes
    };

    row.clear();

    if (in.atEnd())
        return false;

    int state = 0, t;
    QChar ch;
    QString cell;

    while (state >= 0)
    {
        if (in.atEnd())
        {
            t = 4;
        }
        else
        {
            in >> ch;
            if (ch == ',') t = 0;
            else if (ch == '\"') t = 1;
            else if (ch == '\n') t = 2;
            else t = 3;
        }

        state = delta[state][t];
        if (state == 0 || state == 3)
        {
            cell += ch;
        }
        else if (state == -1 || state == 1)
        {
            row.append(cell);
            cell = "";
        }
    }

    return state != -2;
}

TakeoutImportWizard::TakeoutImportWizard(QWidget* parent) : QWizard(parent)
{
    setPage(Page_Intro, new TakeoutImportIntroPage(this));
    setPage(Page_Subs, new TakeoutImportSubsPage(this));
    setPage(Page_WatchHistory, new TakeoutImportWatchHistoryPage(this));
    setPage(Page_Conclusion, new ConclusionPage(this));
    setStartId(Page_Intro);

#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif

    setOption(QWizard::NoCancelButton);
    setWindowTitle("Google Takeout Import Wizard");
}

TakeoutImportIntroPage::TakeoutImportIntroPage(QWidget* parent)
    : QWizardPage(parent),
      info(new QLabel(introInfo, this)),
      layout(new QVBoxLayout(this)),
      subsCheckBox(new QCheckBox("Subscriptions", this)),
      watchHistoryCheckBox(new QCheckBox("Watch History", this))
{
    setTitle("Introduction");

    info->setTextFormat(Qt::RichText);
    info->setWordWrap(true);

    registerField("takeout.intro.watch_history", watchHistoryCheckBox);

    layout->addWidget(info);
    layout->addWidget(subsCheckBox);
    layout->addWidget(watchHistoryCheckBox);

    connect(subsCheckBox, &QCheckBox::clicked, this, &TakeoutImportIntroPage::completeChanged);
    connect(watchHistoryCheckBox, &QCheckBox::clicked, this, &TakeoutImportIntroPage::completeChanged);
}

int TakeoutImportIntroPage::nextId() const
{
    if (subsCheckBox->isChecked())
        return TakeoutImportWizard::Page_Subs;
    if (watchHistoryCheckBox->isChecked())
        return TakeoutImportWizard::Page_WatchHistory;

    return TakeoutImportWizard::Page_Conclusion;
}

TakeoutImportSubsPage::TakeoutImportSubsPage(QWidget* parent)
    : ImportFileSelectPage("Subscriptions", subsSubtitle, "subscriptions.csv",
                           TakeoutImportWizard::Page_ChooseSubs, parent)
{
    connect(this, &ImportFileSelectPage::fileSelected, this, &TakeoutImportSubsPage::verifyFile);
}

void TakeoutImportSubsPage::verifyFile(const QString& fileName)
{
    QFile csv(fileName);
    csv.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&csv);
    if (in.readLine() != "Channel Id,Channel Url,Channel Title")
    {
        pathEdit->setText("Invalid file selected");
        return;
    }

    QList<Entity> outSubs;
    QStringList row;
    while (readCSVRow(in, row))
        if (row[0].startsWith("UC"))
            outSubs.append(Entity(row[0], row[2]));

    pathEdit->setText(fileName);
    emit completeChanged();

    wizard()->setPage(TakeoutImportWizard::Page_ChooseSubs, new ChooseSubsPage(
        outSubs,
        TakeoutImportWizard::Page_Conclusion,
        "takeout.intro.watch_history", TakeoutImportWizard::Page_WatchHistory,
        wizard()
    ));
}

TakeoutImportWatchHistoryPage::TakeoutImportWatchHistoryPage(QWidget* parent)
    : ImportFileSelectPage("Watch History", watchHistorySubtitle, "watch-history.json",
                           TakeoutImportWizard::Page_ChooseWatchHistory, parent)
{
    connect(this, &ImportFileSelectPage::fileSelected, this, &TakeoutImportWatchHistoryPage::verifyFile);
}

void TakeoutImportWatchHistoryPage::verifyFile(const QString& fileName)
{
    QFile json(fileName);
    json.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !jsonDoc.isArray())
    {
        pathEdit->setText("Invalid file selected");
        return;
    }

    QList<Entity> outVideos;
    const QJsonArray videos = jsonDoc.array();
    for (const QJsonValue& entry : videos)
    {
        if (!entry["subtitles"].isArray())
            continue;

        QString id = entry["titleUrl"].toString().remove("https://www.youtube.com/watch?v=");
        QString name = QStringLiteral("<a href=\"%1\">%2</a> by <a href=\"%3\">%4</a>").arg(
            entry["titleUrl"].toString(),
            entry["title"].toString().remove("Watched "),
            entry["subtitles"][0]["url"].toString(),
            entry["subtitles"][0]["name"].toString()
        );
        outVideos.append(Entity(id, name));
    }

    pathEdit->setText(fileName);
    emit completeChanged();

    wizard()->setPage(TakeoutImportWizard::Page_ChooseWatchHistory, new ChooseWatchHistoryPage(
        outVideos, TakeoutImportWizard::Page_Conclusion, wizard()
    ));
}
