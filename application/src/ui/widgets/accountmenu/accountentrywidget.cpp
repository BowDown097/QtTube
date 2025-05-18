#include "accountentrywidget.h"
#include "stores/credentialsstore.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

AccountEntryWidget::AccountEntryWidget(const CredentialSet& credSet, QWidget* parent)
    : ClickableWidget<QWidget>(parent),
      avatarLabel(new TubeLabel(this)),
      layout(new QHBoxLayout(this)),
      nameLabel(new TubeLabel(this))
{
    avatarLabel->setFixedSize(30, 30);
    avatarLabel->setScaledContents(true);
    avatarLabel->setImage(credSet.avatarUrl, TubeLabel::Cached | TubeLabel::Rounded);
    layout->addWidget(avatarLabel);

    nameLabel->setText(credSet.username);
    layout->addWidget(nameLabel);
}
