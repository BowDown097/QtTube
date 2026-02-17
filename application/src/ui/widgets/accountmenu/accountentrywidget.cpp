#include "accountentrywidget.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

AccountEntryWidget::AccountEntryWidget(const QtTubePlugin::AuthUser& user, QWidget* parent)
    : ClickableWidget<>(parent),
      m_avatar(new TubeLabel(this)),
      m_layout(new QHBoxLayout(this)),
      m_nameLabel(new TubeLabel(this))
{
    m_avatar->setFixedSize(30, 30);
    m_avatar->setScaledContents(true);
    m_avatar->setImage(user.avatar, TubeLabel::Cached | TubeLabel::Rounded);
    m_layout->addWidget(m_avatar);

    m_nameLabel->setText(user.username);
    m_layout->addWidget(m_nameLabel);
}
