#include "pipedimportwizard.h"
#include "innertube.h"
#include "shared/choosesubspage.h"
#include "shared/choosewatchhistorypage.h"
#include <QCheckBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLineEdit>
#include <QProgressDialog>

constexpr const char* subsSubtitle = "Select the subscriptions.json file you got from Piped.";
constexpr const char* watchHistorySubtitle = "Select the piped_history_XXX.json file you got from Piped.";

PipedImportWizard::PipedImportWizard(QWidget* parent)
    : DataWizard(Page_Conclusion, "Piped Import Wizard", parent)
{
    setPage(Page_Intro, new PipedImportIntroPage(this));
    setPage(Page_Subs, new PipedImportSubsPage(this));
    setPage(Page_WatchHistory, new PipedImportWatchHistoryPage(this));
    setStartId(Page_Intro);
}

int PipedImportIntroPage::nextId() const
{
    return subsCheckBox->isChecked() ? PipedImportWizard::Page_Subs : PipedImportWizard::Page_WatchHistory;
}

PipedImportSubsPage::PipedImportSubsPage(QWidget* parent)
    : ImportFileSelectPage("Subscriptions", subsSubtitle, "subscriptions.json",
                           PipedImportWizard::Page_ChooseSubs, parent)
{
    connect(this, &ImportFileSelectPage::fileSelected, this, &PipedImportSubsPage::verifyFile);
}

void PipedImportSubsPage::verifyFile(const QString& fileName)
{
    QFile json(fileName);
    json.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !jsonDoc["subscriptions"].isArray())
    {
        pathEdit->setText("Invalid file selected");
        return;
    }

    QList<Entity> outSubs;
    const QJsonArray subs = jsonDoc["subscriptions"].toArray();
    for (const QJsonValue& entry : subs)
    {
        outSubs.append(Entity(
            entry["url"].toString().remove("https://www.youtube.com/channel/"),
            entry["name"].toString()
        ));
    }

    pathEdit->setText(fileName);
    emit completeChanged();

    wizard()->setPage(PipedImportWizard::Page_ChooseSubs, new ChooseSubsPage(
        outSubs, PipedImportWizard::Page_Conclusion, "", 0, wizard()
    ));
}

PipedImportWatchHistoryPage::PipedImportWatchHistoryPage(QWidget* parent)
    : ImportFileSelectPage("Watch History", watchHistorySubtitle, "piped_history*.json",
                           PipedImportWizard::Page_ChooseWatchHistory, parent),
      progressDialog(new QProgressDialog(this))
{
    progressDialog->cancel(); // just prevents from showing automatically
    progressDialog->setLabelText("Getting video data...");
    progressDialog->setWindowModality(Qt::WindowModal);

    connect(this, &ImportFileSelectPage::fileSelected, this, &PipedImportWatchHistoryPage::verifyFile);
    connect(this, &PipedImportWatchHistoryPage::progress, this, &PipedImportWatchHistoryPage::tickProgress);
}

void PipedImportWatchHistoryPage::tickProgress()
{
    int newValue = progressDialog->value() + 1;
    progressDialog->setValue(newValue);
    if (newValue != progressDialog->maximum())
        return;

    emit completeChanged();
    wizard()->setPage(PipedImportWizard::Page_ChooseWatchHistory, new ChooseWatchHistoryPage(
        videos, PipedImportWizard::Page_Conclusion, wizard()
    ));
}

void PipedImportWatchHistoryPage::verifyFile(const QString& fileName)
{
    QFile json(fileName);
    json.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !jsonDoc["playlists"][0]["videos"].isArray())
    {
        pathEdit->setText("Invalid file selected");
        return;
    }

    pathEdit->setText(fileName);

    const QJsonArray videosJson = jsonDoc["playlists"][0]["videos"].toArray();

    progressDialog->reset();
    progressDialog->setMaximum(videosJson.size());
    progressDialog->setValue(0);

    QThreadPool* threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(QThread::idealThreadCount() / 2);

    for (const QJsonValue& entry : videosJson)
    {
        QString id = entry.toString().remove("https://youtube.com/watch?v=");
        threadPool->start([this, id] {
            if (!progressDialog->wasCanceled())
            {
                try
                {
                    auto endpoint = InnerTube::instance()->getBlocking<InnertubeEndpoints::Player>(id);
                    QString name = QStringLiteral("<a href=\"%1\">%2</a> by <a href=\"%3\">%4</a>").arg(
                        "https://www.youtube.com/watch?v=" + id,
                        endpoint.response.videoDetails.title,
                        "https://www.youtube.com/channel/" + endpoint.response.videoDetails.channelId,
                        endpoint.response.videoDetails.author
                    );

                    videos.append(Entity(id, name));

                    // prevent rate limit (apparently it exists but i didn't hit it.. better safe than sorry)
                    QThread::sleep(1);
                }
                catch (const InnertubeException& ie)
                {
                    qWarning() << ie.message();
                }
            }

            emit progress();
        });
    }
}
