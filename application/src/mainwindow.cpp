#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "qttubeapplication.hpp"
#include "ui/browsehelper.hpp"
#include "ui/forms/plugins/pluginbrowserview.hpp"
#include "ui/views/viewcontroller.hpp"
#include "ui/widgets/accountmenu/accountcontrollerwidget.hpp"
#include "ui/widgets/findbar.hpp"
#include "ui/widgets/renderers/browsenotificationrenderer.hpp"
#include "ui/widgets/topbar/topbar.hpp"
#include "utils/uiutils.hpp"
#include <QMessageBox>
#include <qttube-plugin/components/auth/authstore.h>

MainWindow::~MainWindow() { delete ui; }

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QTTUBE_APP_NAME);

    ui->tabWidget->setTopLayout(ui->additionalWidgets);
    ui->centralwidget->widget(0)->addAction(ui->tabWidget->makeReloadAction());

    m_topbar = new TopBar(this);
    m_findbar = new FindBar(this);

    m_notificationMenu = new ContinuableListWidget(this);
    m_notificationMenu->hide();
    m_notificationMenu->setContinuationThreshold(5);

    connect(ui->centralwidget, &QStackedWidget::currentChanged, this, [this] {
        if (m_findbar->isVisible())
            m_findbar->setReveal(false);
    });

    connect(m_topbar->avatarButton, &TubeLabel::clicked, this, &MainWindow::toggleAccountMenu);
    connect(m_topbar->notificationBell, &TopBarBell::clicked, this, &MainWindow::toggleNotificationMenu);
    connect(m_topbar->searchBox, &SearchBox::searchRequested, this, &MainWindow::search);

    connect(m_notificationMenu, &ContinuableListWidget::continuationReady, this, [this] {
        if (m_notificationMenu->continuationData.has_value())
            BrowseHelper::instance()->browseNotificationMenu(m_activePlugin, m_notificationMenu);
    });

    qtTubeApp->doInitialSetup();
    connect(qtTubeApp, &QtTubeApplication::activePluginChanged, this, &MainWindow::activePluginChanged);
    connect(&qtTubeApp->plugins(), &PluginManager::foundUpdate, this, &MainWindow::pluginUpdateAvailable);

#ifdef Q_OS_LINUX
    if (qtTubeApp->settings().playerSettings.vaapi)
    {
        qputenv("LIBVA_DRI3_DISABLE", "1"); // fixes issue on some older GPUs
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", qgetenv("QTWEBENGINE_CHROMIUM_FLAGS") + " --enable-features=VaapiVideoDecoder --enable-features=VaapiIgnoreDriverChecks --disable-features=UseChromeOSDirectVideoDecoder");
    }
#endif

    QCommandLineParser& parser = qtTubeApp->commandLineParser();
    if (PluginEntry* plugin = qtTubeApp->plugins().activePlugin())
    {
        activePluginChanged(plugin);
        if (parser.isSet("channel"))
            ViewController::loadChannel(parser.value("channel"), plugin);
        else if (parser.isSet("video"))
            ViewController::loadVideo(parser.value("video"), plugin);
    }
    else if (parser.isSet("use-plugin"))
    {
        qCritical() << "Plugin not found.";
        exit(EXIT_FAILURE);
    }
    else if (!qtTubeApp->plugins().hasLoadablePlugins())
    {
        // wrapped in this manner to avoid blocking displaying of the main window
        QMetaObject::invokeMethod(this, [this] {
            if (QMessageBox::question(this, "Browse Plugins?", "You have no plugins installed. Would you like to open the plugin browser?") == QMessageBox::Yes)
                PluginBrowserView::spawn();
        }, Qt::QueuedConnection);
    }
}

void MainWindow::activePluginChanged(PluginEntry* activePlugin)
{
    if (QtTubePlugin::AuthStoreBase* authStore = activePlugin->authStore)
    {
        if (const QtTubePlugin::AuthUser* authUser = authStore->activeBaseLogin())
        {
            m_topbar->avatarButton->setImage(authUser->avatar, TubeLabel::Cached | TubeLabel::Rounded);
            m_topbar->postSignInSetup(activePlugin);
        }
        else
        {
            m_topbar->updateUIForSignInState(false);
        }

        connect(authStore, &QtTubePlugin::AuthStoreBase::authenticateSuccess,
                this, &MainWindow::authSucceeded, Qt::UniqueConnection);
        connect(authStore, &QtTubePlugin::AuthStoreBase::updateFail,
                this, &MainWindow::authFailed, Qt::UniqueConnection);
    }
    else
    {
        m_topbar->updateUIForSignInState(false);
    }

    ui->tabWidget->activePluginChanged(activePlugin);
}

void MainWindow::authFailed()
{
    QMessageBox::critical(nullptr, "Invalid Login",
        "Your session has expired or your credentials are invalid. "
        "You will be logged out. Try logging in again.");
    m_topbar->signOut(m_activePlugin);
}

void MainWindow::authSucceeded()
{
    m_topbar->postSignInSetup(m_activePlugin);
}

QStackedWidget* MainWindow::centralWidget()
{
    return ui->centralwidget;
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::PaletteChange)
        ui->tabWidget->repolish();
    QMainWindow::changeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    if ((ctrlPressed && event->key() == Qt::Key_F) || (m_findbar->isVisible() && event->key() == Qt::Key_Escape))
        m_findbar->setReveal(m_findbar->isHidden());

    QMainWindow::keyPressEvent(event);
}

void MainWindow::pluginUpdateAvailable(const QString&, const ReleaseData&)
{
    m_topbar->updateNotificationCount(m_topbar->notificationBell->countText().toInt() + 1);
    // TODO: it would be nice to have this display a system notification as well.
    // that's why some stuff is being sent through the signal. not now though.
}

void MainWindow::reportJsException(const QString& str)
{
    if (ui->centralwidget->currentIndex() == 0)
    {
        if (auto* list = ui->tabWidget->currentWidget()->findChild<ContinuableListWidget*>())
        {
            list->clear();
            list->addItem(str);
            return;
        }
    }

    QMessageBox::critical(this, "JS Error", str, QMessageBox::Ok);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    m_notificationMenu->setFixedSize(width() >= 800 ? 600 : 600 - (800 - width()), height() / 2);
    m_topbar->resize(width(), 35);
    m_topbar->scaleAppropriately();
    m_notificationMenu->move(m_topbar->notificationBell->x() - m_notificationMenu->width() + 20, 34);

    if (auto* accountController = findChild<AccountControllerWidget*>())
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);

    QMainWindow::resizeEvent(event);
}

void MainWindow::search(const QString& query, SearchBox::SearchType searchType)
{
    if (!query.isEmpty())
    {
        if (searchType == SearchBox::SearchType::ByLink)
        {
            qtTubeApp->handleUrlOrID(query);
        }
        else
        {
            m_topbar->setAlwaysShow(true);
            ui->tabWidget->search(query);
        }
    }
}

void MainWindow::toggleAccountMenu()
{
    assert(m_activePlugin != nullptr);

    if (auto* accountController = findChild<AccountControllerWidget*>())
    {
        m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
        accountController->deleteLater();
        return;
    }

    m_topbar->setAlwaysShow(true);

    AccountControllerWidget* accountController = new AccountControllerWidget(m_activePlugin, this);
    accountController->show();
    accountController->raise();
    accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    connect(accountController, &AccountControllerWidget::resized, this, [this, accountController] {
        accountController->move(m_topbar->avatarButton->x() - accountController->width() + 20, 35);
    });
}

void MainWindow::toggleNotificationMenu()
{
    if (m_notificationMenu->isVisible())
    {
        m_topbar->setAlwaysShow(ui->centralwidget->currentIndex() == 0);
        m_notificationMenu->clear();
        m_notificationMenu->hide();
        return;
    }

    m_topbar->setAlwaysShow(true);
    m_notificationMenu->show();

    for (const auto& [name, data] : qtTubeApp->plugins().updatablePlugins())
    {
        BrowseNotificationRenderer* notif = new BrowseNotificationRenderer;
        notif->setData(name, data);
        UIUtils::addWidgetToList(m_notificationMenu, notif);
    }

    if (m_activePlugin && m_activePlugin->providers.notifs && m_activePlugin->authenticated())
        BrowseHelper::instance()->browseNotificationMenu(m_activePlugin, m_notificationMenu);
}

TopBar* MainWindow::topbar()
{
    return m_topbar;
}
