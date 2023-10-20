#ifndef CHOOSEENTITIESPAGE_H
#define CHOOSEENTITIESPAGE_H
#include "ui/forms/settings/data-wizards/entityselecttablemodel.h"
#include "ui/forms/settings/data-wizards/richtableview/richtableview.h"
#include <QCheckBox>
#include <QProgressBar>
#include <QThreadPool>
#include <QVBoxLayout>
#include <QWizardPage>

class ChooseEntitiesPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ChooseEntitiesPage(const QList<Entity>& entities, const QString& title, const QString& subtitle,
                                const QString& checkHeader, const QString& nameHeader, QWidget* parent = nullptr);
    ~ChooseEntitiesPage();

    void initializePage() override;
    bool isComplete() const override { return progressBar->value() == progressBar->maximum(); }
protected:
    bool stopped = false;
    QThreadPool* threadPool;
private:
    QString checkHeader;
    QList<Entity> entities;
    QString nameHeader;

    QHBoxLayout* buttonsLayout;
    QVBoxLayout* layout;
    QProgressBar* progressBar;
    QCheckBox* selectAllCheckBox;
    QPushButton* startButton;
    QPushButton* stopButton;
    RichTableView* table;
private slots:
    void selectAll(bool checked);
    void startTask();
    void stopTask();
    void tickProgress();
signals:
    void foundEntity(const Entity&);
    void progress();
};

#endif // CHOOSEENTITIESPAGE_H
