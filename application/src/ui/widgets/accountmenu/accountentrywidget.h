#pragma once
#include "qttube-plugin/components/auth/authstore.h"
#include "ui/widgets/clickablewidget.h"

class QHBoxLayout;
class TubeLabel;

class AccountEntryWidget : public ClickableWidget<QWidget>
{
    Q_OBJECT
public:
    explicit AccountEntryWidget(const QtTubePlugin::AuthUser& user, QWidget* parent = nullptr);
private:
    TubeLabel* avatarLabel;
    QHBoxLayout* layout;
    TubeLabel* nameLabel;
};
