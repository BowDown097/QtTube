#ifndef ACCOUNTMENUWIDGET_H
#define ACCOUNTMENUWIDGET_H
#include "httpreply.h"
#include "innertube/endpoints/misc/accountmenu.h"
#include "ui/widgets/labels/iconlabel.h"
#include <QLabel>
#include <QVBoxLayout>

class AccountMenuWidget : public QWidget
{
public:
    explicit AccountMenuWidget(QWidget* parent = nullptr);
public slots:
    void initialize(const InnertubeEndpoints::AccountMenu& endpoint);
private:
    QVBoxLayout* accountLayout;
    QLabel* accountNameLabel;
    QLabel* avatar;
    QLabel* handleLabel;
    QHBoxLayout* headerLayout;
    QVBoxLayout* layout;
    IconLabel* signOutLabel;
    IconLabel* switchAccountsLabel;
    IconLabel* yourChannelLabel;
private slots:
    void setAvatar(const HttpReply& reply);
};

#endif // ACCOUNTMENUWIDGET_H
