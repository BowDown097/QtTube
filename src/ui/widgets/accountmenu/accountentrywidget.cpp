#include "accountentrywidget.h"
#include "http.h"
#include "stores/credentialsstore.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>

AccountEntryWidget::AccountEntryWidget(const CredentialSet& credSet, QWidget* parent)
    : QWidget(parent), avatarLabel(new QLabel(this)), layout(new QHBoxLayout(this)), nameLabel(new QLabel(this))
{
    avatarLabel->setFixedSize(30, 30);
    nameLabel->setText(credSet.username);
    layout->addWidget(avatarLabel);
    layout->addWidget(nameLabel);

    HttpReply* reply = Http::instance().get(QUrl(credSet.avatarUrl));
    connect(reply, &HttpReply::finished, this, &AccountEntryWidget::setAvatar);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void AccountEntryWidget::enterEvent(QEnterEvent*)
#else
void AccountEntryWidget::enterEvent(QEvent*)
#endif
{
    if (clickable)
        setCursor(QCursor(Qt::PointingHandCursor));
}

void AccountEntryWidget::leaveEvent(QEvent*)
{
    if (clickable)
        setCursor(QCursor());
}

void AccountEntryWidget::mousePressEvent(QMouseEvent* event)
{
    if (clickable && event->button() == Qt::LeftButton)
        emit clicked();
}

void AccountEntryWidget::setAvatar(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    pixmap = pixmap.scaled(avatarLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    avatarLabel->setPixmap(UIUtils::pixmapRounded(pixmap, 15, 15));
}
