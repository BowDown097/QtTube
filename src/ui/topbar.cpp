#include "../settingsstore.h"
#include "innertube.h"
#include "mainwindow.h"
#include "settingsform.h"
#include "topbar.h"
#include <QJsonDocument>

TopBar::TopBar(QWidget* parent) : QWidget(parent), animation(new QPropertyAnimation(this, "geometry"))
{
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuint);
    resize(MainWindow::instance()->width(), 35);

    QPalette pal{};
    pal.setColor(QPalette::Window, QColor::fromString("#1a1c1e"));
    setAutoFillBackground(true);
    setPalette(pal);

    logo = new ClickableLabel(false, this);
    logo->move(10, 2);
    logo->setPixmap(QPixmap(":/qttube-full.png"));
    logo->resize(134, 30);

    searchBox = new QLineEdit(this);
    searchBox->move(152, 0);
    searchBox->resize(463, 35);

    settingsButton = new QPushButton(this);
    settingsButton->move(623, 0);
    settingsButton->resize(80, 35);
    settingsButton->setText("⚙️");
    connect(settingsButton, &QPushButton::clicked, this, &TopBar::showSettings);

    signInButton = new QPushButton(this);
    signInButton->move(711, 0);
    signInButton->resize(80, 35);
    signInButton->setText("Sign In");
    connect(settingsButton, &QPushButton::clicked, this, &TopBar::trySignIn);
}

TopBar::~TopBar()
{
    delete animation;
    delete searchBox;
    delete signInButton;
    delete settingsButton;
}

void TopBar::showSettings()
{
    SettingsForm* settings = new SettingsForm;
    settings->show();
}

void TopBar::trySignIn()
{
    if (InnerTube::instance().hasAuthenticated())
        return;

    InnerTube::instance().authenticate();
    if (SettingsStore::instance().itcCache)
    {
        QFile store(SettingsStore::configPath.filePath("store.json"));
        if (store.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream storeIn(&store);
            storeIn << QJsonDocument(InnerTube::instance().authStore()->toJson()).toJson(QJsonDocument::Compact);
        }
    }

    signInButton->setText("Sign out");
    emit signedIn();
}
