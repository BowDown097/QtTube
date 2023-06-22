#include "accountmenuwidget.h"
#include "http.h"
#include "ui/forms/mainwindow.h"
#include "ui/views/viewcontroller.h"
#include <QApplication>

AccountMenuWidget::AccountMenuWidget(QWidget* parent)
    : QWidget(parent),
      accountLayout(new QVBoxLayout),
      accountNameLabel(new QLabel(this)),
      avatar(new QLabel(this)),
      handleLabel(new QLabel(this)),
      headerLayout(new QHBoxLayout),
      layout(new QVBoxLayout(this)),
      signOutLabel(new IconLabel("sign-out", "Sign out", QMargins(), QSize(24, 24), this)),
      switchAccountsLabel(new IconLabel("switch-accounts", "Switch account", QMargins(), QSize(24, 24), this)),
      yourChannelLabel(new IconLabel("your-channel", "Your channel", QMargins(), QSize(24, 24), this))
{
    accountNameLabel->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
    avatar->setFixedSize(64, 64);
    setAutoFillBackground(true);

    accountLayout->addWidget(accountNameLabel);
    accountLayout->addWidget(handleLabel);
    accountLayout->addWidget(switchAccountsLabel);

    headerLayout->addWidget(avatar);
    headerLayout->addLayout(accountLayout, 1);

    layout->addLayout(headerLayout);
    layout->addWidget(yourChannelLabel);
    layout->addWidget(signOutLabel);
    layout->addStretch();
    layout->setSizeConstraint(QLayout::SetFixedSize);

    connect(signOutLabel, &IconLabel::clicked, this, [this] {
        setVisible(false);
        MainWindow::topbar()->signOut();
        deleteLater();
    });
}

void AccountMenuWidget::initialize(const InnertubeEndpoints::AccountMenu& endpoint)
{
    accountNameLabel->setText(endpoint.response.header.accountName);
    handleLabel->setText(endpoint.response.header.channelHandle);

    HttpReply* avatarReply = Http::instance().get(QUrl(endpoint.response.header.accountPhotos[0].url));
    connect(avatarReply, &HttpReply::finished, this, &AccountMenuWidget::setAvatar);

    QList<InnertubeObjects::CompactLink>::const_iterator iter = std::ranges::find_if(endpoint.response.sections[0], [](const auto& link) {
        return link.iconType == "ACCOUNT_BOX";
    });

    if (iter != endpoint.response.sections[0].end())
    {
        const InnertubeObjects::CompactLink& link = *iter;
        const QString channelId = link.navigationEndpoint["browseEndpoint"]["browseId"].toString();
        connect(yourChannelLabel, &IconLabel::clicked, this, [this, channelId] {
            setVisible(false);
            ViewController::loadChannel(channelId);
            deleteLater();
        });
    }
}

void AccountMenuWidget::setAvatar(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    avatar->setPixmap(pixmap.scaled(avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
