#ifndef CHOOSEWATCHHISTORYPAGE_H
#define CHOOSEWATCHHISTORYPAGE_H
#include "chooseentitiespage.h"

class ChooseWatchHistoryPage : public ChooseEntitiesPage
{
    Q_OBJECT
public:
    ChooseWatchHistoryPage(const QList<Entity>& videos, int conclusionPage, QWidget* parent = nullptr);
    int nextId() const override { return conclusionPage; }
private:
    int conclusionPage;
private slots:
    void addToWatchHistoryInThread(const Entity& video);
};

#endif // CHOOSEWATCHHISTORYPAGE_H
