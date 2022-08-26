#include "downloadmanager.hpp"
#include "innertube.hpp"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/homevideorenderer.h"
#include "ui/settingsform.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(ui->signInButton, &QPushButton::clicked, this, &MainWindow::signinClicked);
    SettingsStore::instance().initializeFromSettingsFile();
    setupHome();
}

void MainWindow::setupHome()
{
    InnerTube::instance().createContext(InnertubeClient("WEB", "2.20220823.05.00", "DESKTOP", "USER_INTERFACE_THEME_DARK"));
    try
    {
        for (const InnertubeObjects::Video& video : InnerTube::instance().get<InnertubeEndpoints::BrowseHome>().videos)
        {
            HomeVideoRenderer* renderer = new HomeVideoRenderer;
            renderer->setChannelData(video.owner);
            renderer->setVideoData(!video.isLive ? video.lengthText.text : "LIVE", video.publishedTimeText.text, video.title.text, video.videoId, video.viewCountText.text);

            QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
            item->setSizeHint(renderer->sizeHint());
            ui->listWidget->addItem(item);
            ui->listWidget->setItemWidget(item, renderer);

            DownloadManager::instance().append(video.thumbnail.mqdefault);
            connect(&DownloadManager::instance(), &DownloadManager::finishedDownload, renderer, &HomeVideoRenderer::setThumbnail);
        }
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(this, "Failed to get home browsing info", ie.message());
    }
}

void MainWindow::showSettings()
{
    SettingsForm* settings = new SettingsForm;
    settings->show();
}

void MainWindow::signinClicked()
{
    if (InnerTube::instance().hasAuthenticated())
        return;

    InnerTube::instance().authenticate();
    ui->signInButton->setText("Sign out");
    ui->listWidget->clear();
    try
    {
        for (const InnertubeObjects::Video& video : InnerTube::instance().get<InnertubeEndpoints::BrowseHome>().videos)
        {
            HomeVideoRenderer* renderer = new HomeVideoRenderer;
            renderer->setChannelData(video.owner);
            renderer->setVideoData(!video.isLive ? video.lengthText.text : "LIVE", video.publishedTimeText.text, video.title.text, video.videoId, video.viewCountText.text);

            QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
            item->setSizeHint(renderer->sizeHint());
            ui->listWidget->addItem(item);
            ui->listWidget->setItemWidget(item, renderer);

            DownloadManager::instance().append(video.thumbnail.mqdefault);
            connect(&DownloadManager::instance(), &DownloadManager::finishedDownload, renderer, &HomeVideoRenderer::setThumbnail);
        }
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(this, "Failed to get home browsing info", ie.message());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

