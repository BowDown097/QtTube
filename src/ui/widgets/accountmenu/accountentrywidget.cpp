#include "accountentrywidget.h"
#include "stores/credentialsstore.h"
#include "utils/httputils.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QLabel>

AccountEntryWidget::AccountEntryWidget(const CredentialSet& credSet, QWidget* parent)
    : ClickableWidget<QWidget>(parent),
      avatarLabel(new QLabel(this)),
      layout(new QHBoxLayout(this)),
      nameLabel(new QLabel(this))
{
    avatarLabel->setFixedSize(30, 30);
    avatarLabel->setScaledContents(true);
    layout->addWidget(avatarLabel);

    nameLabel->setText(credSet.username);
    layout->addWidget(nameLabel);

    HttpReply* reply = HttpUtils::cachedInstance().get(QUrl(credSet.avatarUrl));
    connect(reply, &HttpReply::finished, this, &AccountEntryWidget::setAvatar);
}

void AccountEntryWidget::setAvatar(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    avatarLabel->setPixmap(UIUtils::pixmapRounded(pixmap));
}
