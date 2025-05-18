#include "accountmenuwidget.h"
#include "innertube/endpoints/misc/accountmenu.h"
#include "mainwindow.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/iconlabel.h"
#include "utils/tubeutils.h"
#include <QBoxLayout>

AccountMenuWidget::AccountMenuWidget(QWidget* parent)
    : QWidget(parent),
      accountLayout(new QVBoxLayout),
      accountNameLabel(new TubeLabel(this)),
      avatar(new TubeLabel(this)),
      handleLabel(new TubeLabel(this)),
      headerLayout(new QHBoxLayout),
      layout(new QVBoxLayout(this)),
      signOutLabel(new IconLabel("sign-out", "Sign out", QMargins(), QSize(24, 24), this)),
      switchAccountsLabel(new IconLabel("switch-accounts", "Switch account", QMargins(), QSize(24, 24), this)),
      yourChannelLabel(new IconLabel("your-channel", "Your channel", QMargins(), QSize(24, 24), this))
{
    setAutoFillBackground(true);

    accountNameLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    accountLayout->addWidget(accountNameLabel);

    accountLayout->addWidget(handleLabel);

    avatar->setFixedSize(48, 48);
    avatar->setScaledContents(true);
    headerLayout->addWidget(avatar);

    headerLayout->addLayout(accountLayout);

    layout->addLayout(headerLayout);
    layout->addWidget(switchAccountsLabel);
    layout->addWidget(yourChannelLabel);
    layout->addWidget(signOutLabel);
    layout->addStretch();
    layout->setSizeConstraint(QLayout::SetFixedSize);

    connect(switchAccountsLabel, &IconLabel::clicked, this, &AccountMenuWidget::accountSwitcherRequested);
    connect(signOutLabel, &IconLabel::clicked, this, &AccountMenuWidget::triggerSignOut);
}

void AccountMenuWidget::initialize(const InnertubeEndpoints::AccountMenu& endpoint)
{
    const InnertubeObjects::ActiveAccountHeader& header = endpoint.response.header;
    accountNameLabel->setText(header.accountName);
    handleLabel->setText(header.channelHandle);

    if (const InnertubeObjects::GenericThumbnail* recAvatar = header.accountPhoto.recommendedQuality(avatar->size()))
        avatar->setImage(recAvatar->url, TubeLabel::Cached | TubeLabel::Rounded);

    QString channelId = TubeUtils::getUcidFromUrl("https://www.youtube.com/" + header.channelHandle);
    connect(yourChannelLabel, &IconLabel::clicked, this, std::bind(&AccountMenuWidget::gotoChannel, this, channelId));

    adjustSize();
    emit finishedInitializing();
}

void AccountMenuWidget::gotoChannel(const QString& channelId)
{
    hide();
    ViewController::loadChannel(channelId);
    emit closeRequested();
}

void AccountMenuWidget::triggerSignOut()
{
    hide();
    MainWindow::topbar()->signOut();
    emit closeRequested();
}
