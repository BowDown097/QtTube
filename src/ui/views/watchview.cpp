#include "watchview.h"
#include "watchview_ui.h"
#include "http.h"
#include "innertube.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/forms/livechat/livechatwindow.h"
#include "utils/osutils.h"
#include "utils/uiutils.h"
#include <QDesktopServices>
#include <QScrollBar>

WatchView::~WatchView()
{
    delete ui;
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, nullptr);
    OSUtils::toggleIdleSleep(false);
}

WatchView::WatchView(const QString& videoId, int progress, QWidget* parent) : QWidget(parent), ui(new Ui::WatchView)
{
    OSUtils::toggleIdleSleep(true);
    MainWindow::topbar()->setVisible(false);
    MainWindow::topbar()->alwaysShow = false;

    ui->setupUi(this);

    InnertubeReply* next = InnerTube::instance().get<InnertubeEndpoints::Next>(videoId);
    connect(next, qOverload<const InnertubeEndpoints::Next&>(&InnertubeReply::finished), this, &WatchView::processNext);
    connect(next, &InnertubeReply::exception, this, &WatchView::loadFailed);

    InnertubeReply* player = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId);
    connect(player, qOverload<const InnertubeEndpoints::Player&>(&InnertubeReply::finished), this, &WatchView::processPlayer);
    connect(player, &InnertubeReply::exception, this, &WatchView::loadFailed);

    ui->player->play(videoId, progress);
    connect(ui->channelLabel->text, &TubeLabel::customContextMenuRequested, this, &WatchView::showContextMenu);
    connect(ui->description, &TubeLabel::linkActivated, this, &WatchView::descriptionLinkActivated);
}

void WatchView::descriptionLinkActivated(const QString& url)
{
    QUrl qUrl(url);
    if (url.startsWith("http"))
    {
        QDesktopServices::openUrl(qUrl);
    }
    else if (url.startsWith("/channel"))
    {
        QString funnyPath = qUrl.path().replace("/channel/", "");
        emit navigateChannelRequested(funnyPath.left(funnyPath.indexOf('/')));
    }
    else if (url.startsWith("/watch"))
    {
        ui->scrollArea->verticalScrollBar()->setValue(0);
        QUrlQuery query(qUrl);
        if (query.queryItemValue("continuePlayback") == "1")
        {
            int progress = query.queryItemValue("t").replace("s", "").toInt();
            ui->player->seek(progress);
        }
        else
        {
            hotLoadVideo(query.queryItemValue("v"), query.queryItemValue("t").toInt());
            ui->toggleShowMore();
        }
    }
    else
    {
        qDebug() << "Ran into unsupported description link:" << url;
    }
}

QString WatchView::generateFormattedDescription(const InnertubeObjects::InnertubeString& description)
{
    QString descriptionText;

    for (const InnertubeObjects::InnertubeRun& run : description.runs)
    {
        if (run.navigationEndpoint.isNull() || run.navigationEndpoint.isUndefined())
        {
            descriptionText += run.text;
            continue;
        }

        QString href;
        if (run.navigationEndpoint["urlEndpoint"].isObject())
        {
            QUrl url(run.navigationEndpoint["urlEndpoint"]["url"].toString());
            QUrlQuery query(url);
            if (query.hasQueryItem("q"))
                href = QUrl::fromPercentEncoding(query.queryItemValue("q").toUtf8());
            else if (url.host() == "www.youtube.com" && url.path().startsWith("/channel"))
                href = url.path();
            else
                href = url.toString();
        }
        else
        {
            href = run.navigationEndpoint["commandMetadata"]["webCommandMetadata"]["url"].toString();
            if (run.navigationEndpoint["watchEndpoint"].isObject())
                href += "&continuePlayback=" + QString::number(run.navigationEndpoint["watchEndpoint"]["continuePlayback"].toBool());
        }

        descriptionText += QStringLiteral("<a href=\"%1\">%2</a>").arg(href, run.text);
    }

    return descriptionText.replace("\n", "<br>");
}

void WatchView::hotLoadVideo(const QString& videoId, int progress)
{
    ui->player->stopTracking();
    if (metadataUpdateTimer)
        metadataUpdateTimer->deleteLater();

    UIUtils::clearLayout(ui->topLevelButtons);
    disconnect(ui->channelLabel->text, &TubeLabel::clicked, nullptr, nullptr);

    InnertubeReply* next = InnerTube::instance().get<InnertubeEndpoints::Next>(videoId);
    connect(next, qOverload<const InnertubeEndpoints::Next&>(&InnertubeReply::finished), this, &WatchView::processNext);
    connect(next, &InnertubeReply::exception, this, &WatchView::loadFailed);

    InnertubeReply* player = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId);
    connect(player, qOverload<const InnertubeEndpoints::Player&>(&InnertubeReply::finished), this, &WatchView::processPlayer);
    connect(player, &InnertubeReply::exception, this, &WatchView::loadFailed);

    ui->player->play(videoId, progress);
}

void WatchView::likeOrDislike(bool like, const InnertubeObjects::ToggleButton& toggleButton)
{
    IconLabel* senderLabel = qobject_cast<IconLabel*>(sender());

    bool textIsNumber;
    qint64 count = QLocale::system().toLongLong(senderLabel->text(), &textIsNumber);

    if (senderLabel->styleSheet().isEmpty()) // if untoggled
    {
        senderLabel->setIcon(like ? "like-toggled" : "dislike-toggled");
        senderLabel->setStyleSheet("color: #167ac6");
        if (textIsNumber)
            senderLabel->setText(QLocale::system().toString(count + 1));

        const QJsonArray defaultCommands = toggleButton.defaultServiceEndpoint["commandExecutorCommand"]["commands"].toArray();
        auto defaultCommandIt = std::ranges::find_if(defaultCommands, [](const QJsonValue& v) { return v["commandMetadata"].isObject(); });
        if (defaultCommandIt == defaultCommands.end())
            return;

        const QJsonValue& defaultCommand = *defaultCommandIt;
        InnerTube::instance().like(defaultCommand["likeEndpoint"], like);
    }
    else
    {
        senderLabel->setIcon(like ? "like" : "dislike");
        senderLabel->setStyleSheet(QString());
        if (textIsNumber)
            senderLabel->setText(QLocale::system().toString(count - 1));

        InnerTube::instance().like(toggleButton.toggledServiceEndpoint["likeEndpoint"], like);
    }
}

void WatchView::processNext(const InnertubeEndpoints::Next& endpoint)
{
    InnertubeEndpoints::NextResponse nextResp = endpoint.response;
    channelId = nextResp.secondaryInfo.owner.navigationEndpoint["browseEndpoint"]["browseId"].toString();

    ui->channelLabel->setInfo(nextResp.secondaryInfo.owner.title.text, nextResp.secondaryInfo.owner.badges);
    connect(ui->channelLabel->text, &TubeLabel::clicked, this, std::bind(&WatchView::navigateChannelRequested, this, channelId));

    ui->subscribeWidget->setSubscribeButton(nextResp.secondaryInfo.subscribeButton);
    ui->subscribeWidget->setSubscriberCount(nextResp.secondaryInfo.owner.subscriberCountText.text, nextResp.secondaryInfo.subscribeButton.channelId);
    ui->subscribeWidget->subscribersCountLabel->setVisible(true);
    ui->viewCount->setText(qtTubeApp->settings().condensedCounts && !nextResp.primaryInfo.viewCount.isLive
                               ? nextResp.primaryInfo.viewCount.extraShortViewCount.text + " views"
                               : nextResp.primaryInfo.viewCount.viewCount.text);

    for (const InnertubeObjects::MenuFlexibleItem& fi : nextResp.primaryInfo.videoActions.flexibleItems)
    {
        // these will never be implemented
        if (fi.topLevelButton.iconType == "CONTENT_CUT" || fi.topLevelButton.iconType == "MONEY_HEART")
            continue;

        // change "Save" button to "Add to" to replicate Hitchhiker style
        QString labelText = fi.topLevelButton.text.text;
        if (labelText == "Save")
            labelText = "Add to";

        ui->topLevelButtons->addWidget(new IconLabel(
            fi.topLevelButton.iconType.toLower(),
            labelText,
            ui->topLevelButtons->count() > 0 ? QMargins(15, 0, 0, 0) : QMargins(5, 0, 0, 0)
        ));
    }

    IconLabel* shareLabel = new IconLabel("share", "Share", ui->topLevelButtons->count() > 0 ? QMargins(15, 0, 0, 0) : QMargins(5, 0, 0, 0));
    ui->topLevelButtons->addWidget(shareLabel);

    if (nextResp.liveChat.has_value())
    {
        IconLabel* liveChatLabel = new IconLabel("live-chat", "Chat", QMargins(15, 0, 0, 0));
        ui->topLevelButtons->addWidget(liveChatLabel);

        connect(liveChatLabel, &IconLabel::clicked, this, [this, nextResp]
        {
            LiveChatWindow* liveChatWindow = new LiveChatWindow;
            liveChatWindow->show();
            liveChatWindow->initialize(nextResp.liveChat.value(), ui->player);
        });
    }

    ui->topLevelButtons->addStretch();

    ui->likeLabel = new IconLabel("like", QMargins(0, 0, 15, 0));
    ui->topLevelButtons->addWidget(ui->likeLabel);
    connect(ui->likeLabel, &IconLabel::clicked, this, std::bind(&WatchView::likeOrDislike, this, true, nextResp.primaryInfo.videoActions.likeButton));
    if (nextResp.primaryInfo.videoActions.likeButton.isToggled)
    {
        ui->likeLabel->setIcon("like-toggled");
        ui->likeLabel->setStyleSheet("color: #167ac6");
    }

    ui->dislikeLabel = new IconLabel("dislike");
    ui->topLevelButtons->addWidget(ui->dislikeLabel);
    connect(ui->dislikeLabel, &IconLabel::clicked, this, std::bind(&WatchView::likeOrDislike, this, false, nextResp.primaryInfo.videoActions.dislikeButton));
    if (nextResp.primaryInfo.videoActions.dislikeButton.isToggled)
    {
        ui->dislikeLabel->setIcon("dislike-toggled");
        ui->dislikeLabel->setStyleSheet("color: #167ac6");
    }

    if (!nextResp.secondaryInfo.owner.thumbnails.isEmpty())
    {
        QList<InnertubeObjects::GenericThumbnail>::iterator bestThumb = std::ranges::max_element(
            nextResp.secondaryInfo.owner.thumbnails,
            [](const auto& a, const auto& b) { return a.width < b.width; }
        );
        HttpReply* reply = Http::instance().get(bestThumb->url);
        connect(reply, &HttpReply::finished, this, &WatchView::setChannelIcon);
    }

    InnertubeObjects::InnertubeString likeText = nextResp.primaryInfo.videoActions.likeButton.defaultText;
    ui->likeLabel->setText(qtTubeApp->settings().condensedCounts ? likeText.text : UIUtils::extractDigits(likeText.accessibilityLabel));

    if (qtTubeApp->settings().returnDislikes)
    {
        HttpReply* reply = Http::instance().get("https://returnyoutubedislikeapi.com/votes?videoId=" + nextResp.videoId);
        connect(reply, &HttpReply::finished, this, &WatchView::setDislikes);
    }
    else
    {
        ui->dislikeLabel->setText("Dislike");
    }

    // Add "Published on" before date of normal videos to replicate Hitchhiker style
    QString dateText = nextResp.primaryInfo.dateText.text;
    if (!dateText.startsWith("Premier") && !dateText.startsWith("St") && !dateText.startsWith("Sched"))
        dateText.prepend("Published on ");

    ui->date->setText(dateText);
    ui->description->setText(generateFormattedDescription(nextResp.secondaryInfo.description));
}

void WatchView::processPlayer(const InnertubeEndpoints::Player& endpoint)
{
    InnertubeEndpoints::PlayerResponse playerResp = endpoint.response;
    ui->player->startTracking(playerResp);
    ui->titleLabel->setText(playerResp.videoDetails.title);

    if (playerResp.videoDetails.isLive)
    {
        metadataUpdateTimer = new QTimer(this);
        metadataUpdateTimer->setInterval(60000);
        connect(metadataUpdateTimer, &QTimer::timeout, this, [playerResp, this]
        {
            try
            {
                auto updatedMetadata = InnerTube::instance().getBlocking<InnertubeEndpoints::UpdatedMetadata>(playerResp.videoDetails.videoId);
                updateMetadata(updatedMetadata.response);
            }
            catch (const InnertubeException&)
            {
                qDebug() << "InnertubeException on UpdateMetadata. Stream/premiere likely ended. Killing update timer.";
                metadataUpdateTimer->deleteLater();
            }
        });
        metadataUpdateTimer->start();
    }
}

void WatchView::resizeEvent(QResizeEvent* event)
{
    if (!ui->primaryInfoWrapper || !event->oldSize().isValid())
        return;

    ui->player->calcAndSetSize(size());
    ui->scrollArea->setFixedSize(event->size());

    ui->description->setFixedWidth(ui->player->size().width());
    ui->menuWrapper->setFixedWidth(ui->player->size().width());
    ui->primaryInfoWrapper->setFixedWidth(ui->player->size().width());
    ui->showMoreLabel->setFixedWidth(ui->player->size().width());
    ui->titleLabel->setFixedWidth(ui->player->size().width());
}

void WatchView::setChannelIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    ui->channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WatchView::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, std::bind(&UIUtils::copyToClipboard, "https://www.youtube.com/channel/" + channelId));

    menu->addAction(copyUrlAction);
    menu->popup(ui->channelLabel->text->mapToGlobal(pos));
}

void WatchView::updateMetadata(const InnertubeEndpoints::UpdatedMetadataResponse& resp)
{
    ui->date->setText(resp.dateText);
    ui->description->setText(generateFormattedDescription(resp.description));
    ui->likeLabel->setText(qtTubeApp->settings().condensedCounts ? resp.likeDefaultText : QLocale::system().toString(resp.likeNumericalValue.toLongLong()));
    ui->titleLabel->setText(resp.title.text);
    ui->viewCount->setText(resp.viewCount.viewCount.text);

    if (qtTubeApp->settings().returnDislikes)
    {
        HttpReply* reply = Http::instance().get("https://returnyoutubedislikeapi.com/votes?videoId=" + resp.videoId);
        connect(reply, &HttpReply::finished, this, &WatchView::setDislikes);
    }
}

void WatchView::setDislikes(const HttpReply& reply)
{
    QJsonDocument doc = QJsonDocument::fromJson(reply.body());
    qint64 dislikes = doc["dislikes"].toVariant().toLongLong();
    qint64 likes = QLocale::system().toLongLong(ui->likeLabel->text());

    if (likes != 0)
    {
        ui->likeBar->setMaximum(likes + dislikes);
        ui->likeBar->setValue(likes);
    }

    ui->likeBar->setVisible(true);

#ifdef QTTUBE_HAS_ICU
    if (qtTubeApp->settings().condensedCounts)
    {
        ui->dislikeLabel->setText(UIUtils::condensedNumericString(dislikes));
        return;
    }
#endif

    ui->dislikeLabel->setText(QLocale::system().toString(dislikes));
}
