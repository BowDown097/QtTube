#pragma once
#include "ui/widgets/clickablewidget.h"

struct CredentialSet;
class HttpReply;
class QHBoxLayout;
class QLabel;

class AccountEntryWidget : public ClickableWidget<QWidget>
{
    Q_OBJECT
public:
    explicit AccountEntryWidget(const CredentialSet& credSet, QWidget* parent = nullptr);
private:
    QLabel* avatarLabel;
    QHBoxLayout* layout;
    QLabel* nameLabel;
private slots:
    void setAvatar(const HttpReply& reply);
};
