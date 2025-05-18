#pragma once
#include "ui/widgets/clickablewidget.h"

struct CredentialSet;
class QHBoxLayout;
class TubeLabel;

class AccountEntryWidget : public ClickableWidget<QWidget>
{
    Q_OBJECT
public:
    explicit AccountEntryWidget(const CredentialSet& credSet, QWidget* parent = nullptr);
private:
    TubeLabel* avatarLabel;
    QHBoxLayout* layout;
    TubeLabel* nameLabel;
};
