#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qttubeapplication.h"
#include "stores/settingsstore.h"
#include "ui/browsehelper.h"
#include "ui/views/channelview.h"
#include "ui/views/viewcontroller.h"
#include "ui/views/watchview.h"
#include "ui/widgets/accountmenu/accountcontrollerwidget.h"
#include "utils/uiutils.h"
#include <QAction>
#include <QComboBox>
#include <QLineEdit>
#include <QScrollBar>
#include <QUrlQuery>

MainWindow::~MainWindow() { delete ui; }

MainWindow::MainWindow(const QCommandLineParser& parser, QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QTTUBE_APP_NAME);

    m_centralWidget = ui->centralwidget;
    m_size = geometry().size();
    m_topbar = new TopBar(this);

    notificationMenu = new ContinuableListWidget(this);
    notificationMenu->hide();
    notificationMenu->setContinuationThreshold(5);

    findbar = new FindBar(this);
    connect(ui->centralwidget, &QStackedWidget::currentChanged, this, [this] {
        if (findbar->isVisible())
            findbar->setReveal(false);
    });

    connect(m_topbar, &TopBar::signInStatusChanged, this, [this] { if (ui->centralwidget->currentIndex() == 0) browse(); });
    connect(m_topbar->avatarButton, &TubeLabel::clicked, this, &MainWindow::showAccountMenu);
    connect(m_topbar->notificationBell, &TopBarBell::clicked, this, &MainWindow::showNotifications);
    connect(m_topbar->searchBox, &SearchBox::searchRequested, this, &MainWindow::search);

    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setCurrentIndex(5); // just some blank tab so you can pick one
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::browse);

    connect(notificationMenu, &ContinuableListWidget::continuationReady, this, [this] {
        BrowseHelper::instance()->continuation<InnertubeEndpoints::GetNotificationMenu>(notificationMenu, "NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX", 5);
    });
    connect(ui->historyWidget, &ContinuableListWidget::continuationReady, this, [this] {
        BrowseHelper::instance()->continuation<InnertubeEndpoints::BrowseHistory>(ui->historyWidget, lastSearchQuery);
    });
    connect(ui->homeWidget, &ContinuableListWidget::continuationReady, this, [this] {
        BrowseHelper::instance()->continuation<InnertubeEndpoints::BrowseHome>(ui->homeWidget);
    });
    connect(ui->searchWidget, &ContinuableListWidget::continuationReady, this, [this] {
        BrowseHelper::instance()->continuation<InnertubeEndpoints::Search>(ui->searchWidget, lastSearchQuery);
    });
    connect(ui->subscriptionsWidget, &ContinuableListWidget::continuationReady, this, [this] {
        BrowseHelper::instance()->continuation<InnertubeEndpoints::BrowseSubscriptions>(ui->subscriptionsWidget);
    });

    QAction* reloadShortcut = new QAction(this);
    reloadShortcut->setAutoRepeat(false);
    reloadShortcut->setShortcuts(QList<QKeySequence>() << Qt::Key_F5 << QKeySequence(Qt::ControlModifier | Qt::Key_R));
    connect(reloadShortcut, &QAction::triggered, this, &MainWindow::reloadCurrentTab);
    addAction(reloadShortcut);

    connect(InnerTube::instance()->authStore(), &InnertubeAuthStore::authenticateSuccess, this, [this] {
        m_topbar->postSignInSetup();
    });

    qtTubeApp->doInitialSetup();

#ifdef Q_OS_LINUX
    // Setting has no effect on Wayland because VAAPI accel
    // unfortunately causes a crash on it for now.
    if (qtTubeApp->settings().vaapi && qApp->platformName() != "wayland")
    {
        qputenv("LIBVA_DRI3_DISABLE", "1"); // fixes issue on some older GPUs
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", qgetenv("QTWEBENGINE_CHROMIUM_FLAGS") + " --enable-features=VaapiVideoDecoder --enable-features=VaapiIgnoreDriverChecks --disable-features=UseChromeOSDirectVideoDecoder");
    }
#endif

    if (parser.isSet("channel"))
        ViewController::loadChannel(parser.value("channel"));
    else if (parser.isSet("video"))
        ViewController::loadVideo(parser.value("video"));
}

void MainWindow::browse()
{
    if (doNotBrowse)
        return;

    UIUtils::clearLayout(ui->additionalWidgets);
    ui->historySearchWidget->clear();
    ui->historyWidget->clear();
    ui->homeWidget->clear();
    ui->searchWidget->clear();
    ui->subscriptionsWidget->clear();
    ui->trendingWidget->clear();

    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        ui->homeWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseHome(ui->homeWidget);
        break;
    case 1:
        ui->trendingWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseTrending(ui->trendingWidget);
        break;
    case 2:
        ui->subscriptionsWidget->toggleListGridLayout();
        BrowseHelper::instance()->browseSubscriptions(ui->subscriptionsWidget);
        break;
    case 3:
        QLineEdit* historySearch = new QLineEdit(this);
        historySearch->setPlaceholderText("Search watch history");
        ui->additionalWidgets->addWidget(historySearch);
        connect(historySearch, &QLineEdit::returnPressed, this, &MainWindow::searchWatchHistory);

        BrowseHelper::instance()->browseHistory(ui->historyWidget);
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    if ((ctrlPressed && event->key() == Qt::Key_F) || (findbar->isVisible() && event->key() == Qt::Key_Escape))
        findbar->setReveal(findbar->isHidden());

    QMainWindow::keyPressEvent(event);
}

void MainWindow::performFilteredSearch()
{
    ui->searchWidget->clear();
    int dateIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(1)->widget())->currentIndex();
    int typeIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(2)->widget())->currentIndex();
    int durIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(3)->widget())->currentIndex();
    int featIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(4)->widget())->currentIndex();
    int sortIndex = qobject_cast<QComboBox*>(ui->additionalWidgets->itemAt(5)->widget())->currentIndex();
    BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery, dateIndex, typeIndex, durIndex, featIndex, sortIndex);
}

void MainWindow::reloadCurrentTab()
{
    if (ui->centralwidget->currentIndex() != 0 || !ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
        return;

    if (QWidget* widget = ui->tabWidget->widget(ui->tabWidget->currentIndex()))
        if (ContinuableListWidget* list = widget->findChild<ContinuableListWidget*>(); list->isPopulating())
            return;

    if (ui->tabWidget->currentIndex() <= 3)
        browse();
    else if (ui->tabWidget->currentIndex() == 4)
        performFilteredSearch();
    else if (ui->tabWidget->currentIndex() == 5)
        searchWatchHistory();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    m_size = event->size();
    notificationMenu->setFixedSize(width() >= 800 ? 600 : 600 - (800 - width()), height() / 2);
    m_topbar->resize(width(), 35);
    m_topbar->scaleAppropriately();
    notificationMenu->move(m_topbar->notificationBell->x() - notificationMenu->width() + 20, 34);

    if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>("accountController"))
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);

    QMainWindow::resizeEvent(event);
}

void MainWindow::returnFromSearch()
{
    UIUtils::clearLayout(ui->additionalWidgets);
    doNotBrowse = true;
    disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
    ui->tabWidget->setTabEnabled(4, false);
    UIUtils::setTabsEnabled(ui->tabWidget, true, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(0);
    ui->searchWidget->clear();
}

void MainWindow::returnFromWatchHistorySearch()
{
    doNotBrowse = true;
    disconnect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
    ui->tabWidget->setTabEnabled(5, false);
    UIUtils::setTabsEnabled(ui->tabWidget, true, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(3);
    ui->historySearchWidget->clear();
}

void MainWindow::search(const QString& query, SearchBox::SearchType searchType)
{
    if (query.isEmpty())
        return;

    if (searchType == SearchBox::SearchType::ByLink)
        searchByLink(query);
    else
        searchByQuery(query);
}

void MainWindow::searchByLink(const QString& link)
{
    static QRegularExpression channelRegex(R"((?:^|\/channel\/)(UC[a-zA-Z0-9_-]{22})(?=\b))");
    static QRegularExpression videoRegex(R"((?:^|v=|vi=|v\/|vi\/|shorts\/|live\/|youtu\.be\/)([a-zA-Z0-9_-]{11})(?=\b))");

    if (QRegularExpressionMatch channelMatch = channelRegex.match(link); channelMatch.lastCapturedIndex() >= 1)
    {
        ViewController::loadChannel(channelMatch.captured(1));
    }
    else if (QRegularExpressionMatch videoMatch = videoRegex.match(link); videoMatch.lastCapturedIndex() >= 1)
    {
        int progress{};
        if (QUrl url(link); url.isValid())
            if (QUrlQuery urlQuery(url); urlQuery.hasQueryItem("t"))
                progress = urlQuery.queryItemValue("t").toInt();

        ViewController::loadVideo(videoMatch.captured(1), progress);
    }
    else
    {
        // this hits all kinds of stuff, but we're going to specifically filter for channels and videos.
        // doesn't have to be an exact URL either, so just giving a handle and stuff like that should still work
        using UrlEndpoint = InnertubeEndpoints::ResolveUrl;
        using UrlReply = InnertubeReply<UrlEndpoint>;
        auto reply = InnerTube::instance()->get<UrlEndpoint>(link);
        connect(reply, &UrlReply::exception, this, [this, link](const InnertubeException& ex) {
            QMessageBox::critical(this, "Error", ex.message());
        });
        connect(reply, &UrlReply::finished, this, [this](const UrlEndpoint& endpoint) {
            QString webPageType = endpoint.endpoint["commandMetadata"]["webCommandMetadata"]["webPageType"].toString();
            if (webPageType == "WEB_PAGE_TYPE_CHANNEL")
            {
                ViewController::loadChannel(endpoint.endpoint["browseEndpoint"]["browseId"].toString());
            }
            else if (webPageType == "WEB_PAGE_TYPE_WATCH")
            {
                ViewController::loadVideo(
                    endpoint.endpoint["watchEndpoint"]["videoId"].toString(),
                    endpoint.endpoint["watchEndpoint"]["startTimeSeconds"].toInt());
            }
            else
            {
                // check for an edge case where a classic channel URL is returned instead of the UCID
                if (QString classicUrl = endpoint.endpoint["urlEndpoint"]["url"].toString(); !classicUrl.isEmpty())
                {
                    auto reply2 = InnerTube::instance()->get<UrlEndpoint>(classicUrl);
                    connect(reply2, &UrlReply::finished, this, [](const UrlEndpoint& endpoint2) {
                        if (endpoint2.endpoint["commandMetadata"]["webCommandMetadata"]["webPageType"].toString() == "WEB_PAGE_TYPE_CHANNEL")
                            ViewController::loadChannel(endpoint2.endpoint["browseEndpoint"]["browseId"].toString());
                    });
                }
                else
                {
                    QMessageBox::warning(this, "Nothing Found!", "Could not find anything from your input.");
                }
            }
        });
    }
}

void MainWindow::searchByQuery(const QString& query)
{
    m_topbar->setAlwaysShow(true);
    UIUtils::clearLayout(ui->additionalWidgets);
    ui->historySearchWidget->clear();

    if (ChannelView* channelView = qobject_cast<ChannelView*>(ui->centralwidget->currentWidget()))
        channelView->deleteLater();
    else if (WatchView* watchView = qobject_cast<WatchView*>(ui->centralwidget->currentWidget()))
        watchView->deleteLater();

    TubeLabel* filtersLabel = new TubeLabel("Filters:", this);
    ui->additionalWidgets->addWidget(filtersLabel);

    QComboBox* dateCmb = new QComboBox(this);
    dateCmb->setPlaceholderText("Upload date");
    dateCmb->addItems({"Last hour", "Today", "This week", "This month", "This year"});
    ui->additionalWidgets->addWidget(dateCmb);

    QComboBox* typeCmb = new QComboBox(this);
    typeCmb->setPlaceholderText("Type");
    typeCmb->addItems({"Video", "Channel", "Playlist", "Movie"});
    ui->additionalWidgets->addWidget(typeCmb);

    QComboBox* durCmb = new QComboBox(this);
    durCmb->setPlaceholderText("Duration");
    durCmb->addItems({"Under 4 minutes", "Over 20 minutes", "4-20 minutes"});
    ui->additionalWidgets->addWidget(durCmb);

    QComboBox* featCmb = new QComboBox(this);
    featCmb->setPlaceholderText("Features");
    featCmb->addItems({"Live", "4K", "HD", "Subtitles/CC", "Creative Commons", "360°", "VR180", "3D", "HDR", "Location", "Purchased"});
    ui->additionalWidgets->addWidget(featCmb);

    QComboBox* sortCmb = new QComboBox(this);
    sortCmb->setPlaceholderText("Sort by");
    sortCmb->addItems({"Relevance", "Rating", "Upload date", "View count"});
    ui->additionalWidgets->addWidget(sortCmb);

    if (ui->tabWidget->currentIndex() == 4)
    {
        ui->searchWidget->clear();
    }
    else
    {
        doNotBrowse = true;
        connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromSearch);
        ui->tabWidget->setTabEnabled(4, true);
        UIUtils::setTabsEnabled(ui->tabWidget, false, {0, 1, 2, 3, 5});
        doNotBrowse = false;
        ui->tabWidget->setCurrentIndex(4);
    }

    lastSearchQuery = query;
    BrowseHelper::instance()->search(ui->searchWidget, lastSearchQuery);

    connect(dateCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(typeCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(durCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(featCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
    connect(sortCmb, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::performFilteredSearch);
}

void MainWindow::searchWatchHistory()
{
    if (ui->tabWidget->currentIndex() == 5)
    {
        ui->historySearchWidget->clear();
        lastSearchQuery = qobject_cast<QLineEdit*>(ui->additionalWidgets->itemAt(0)->widget())->text();
        BrowseHelper::instance()->browseHistory(ui->historySearchWidget, lastSearchQuery);
        return;
    }

    doNotBrowse = true;
    connect(m_topbar->logo, &TubeLabel::clicked, this, &MainWindow::returnFromWatchHistorySearch);
    ui->tabWidget->setTabEnabled(5, true);
    UIUtils::setTabsEnabled(ui->tabWidget, false, {0, 1, 2, 3});
    doNotBrowse = false;
    ui->tabWidget->setCurrentIndex(5);

    lastSearchQuery = qobject_cast<QLineEdit*>(ui->additionalWidgets->itemAt(0)->widget())->text();
    BrowseHelper::instance()->browseHistory(ui->historySearchWidget, lastSearchQuery);
}

void MainWindow::showAccountMenu()
{
    if (AccountControllerWidget* accountController = findChild<AccountControllerWidget*>())
    {
        m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
        accountController->deleteLater();
        return;
    }

    m_topbar->setAlwaysShow(true);

    AccountControllerWidget* accountController = new AccountControllerWidget(this);
    accountController->show();
    accountController->raise();
    accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    connect(accountController, &AccountControllerWidget::resized, this, [this, accountController] {
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    });

    auto reply = InnerTube::instance()->get<InnertubeEndpoints::AccountMenu>();
    connect(reply, &InnertubeReply<InnertubeEndpoints::AccountMenu>::finished, accountController->accountMenu, &AccountMenuWidget::initialize);
}

void MainWindow::showNotifications()
{
    if (notificationMenu->isVisible())
    {
        m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
        notificationMenu->clear();
        notificationMenu->hide();
        return;
    }

    m_topbar->setAlwaysShow(true);
    notificationMenu->show();
    BrowseHelper::instance()->browseNotificationMenu(notificationMenu);
}
