#pragma once
#include "qttube-plugin/components/auth/authstore.h"
#include "ui/widgets/clickablewidget.h"

class QHBoxLayout;
class TubeLabel;

class AccountEntryWidget : public ClickableWidget<>
{
    Q_OBJECT
public:
    explicit AccountEntryWidget(const QtTubePlugin::AuthUser& user, QWidget* parent = nullptr);
private:
    TubeLabel* m_avatar;
    QHBoxLayout* m_layout;
    TubeLabel* m_nameLabel;
};
