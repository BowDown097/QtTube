#ifndef CHOOSESUBSPAGE_H
#define CHOOSESUBSPAGE_H
#include "chooseentitiespage.h"

class ChooseSubsPage : public ChooseEntitiesPage
{
    Q_OBJECT
public:
    ChooseSubsPage(const QList<Entity>& subs, int conclusionPage, const QString& watchHistoryKey = "",
                   int watchHistoryPage = 0, QWidget* parent = nullptr);
    int nextId() const override { return field(watchHistoryKey).toBool() ? watchHistoryPage : conclusionPage; }
private:
    int conclusionPage;
    QString watchHistoryKey;
    int watchHistoryPage;
private slots:
    void subToChannelInThread(const Entity& channel);
};

#endif // CHOOSESUBSPAGE_H
