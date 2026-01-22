#include "accountentrywidget.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

AccountEntryWidget::AccountEntryWidget(const QtTubePlugin::AuthUser& user, QWidget* parent)
    : ClickableWidget<>(parent),
      avatarLabel(new TubeLabel(this)),
      layout(new QHBoxLayout(this)),
      nameLabel(new TubeLabel(this))
{
    avatarLabel->setFixedSize(30, 30);
    avatarLabel->setScaledContents(true);
    avatarLabel->setImage(user.avatar, TubeLabel::Cached | TubeLabel::Rounded);
    layout->addWidget(avatarLabel);

    nameLabel->setText(user.username);
    layout->addWidget(nameLabel);
}
