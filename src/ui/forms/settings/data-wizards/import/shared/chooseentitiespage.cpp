#include "chooseentitiespage.h"
#include "ui/forms/settings/data-wizards/richtableview/richitemdelegate.h"
#include "ui/forms/settings/data-wizards/richtableview/richtableview.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QHeaderView>
#include <QProgressBar>
#include <QPushButton>
#include <QThreadPool>

ChooseEntitiesPage::ChooseEntitiesPage(const QList<Entity>& entities, const QString& title, const QString& subtitle,
                                       const QString& checkHeader, const QString& nameHeader, QWidget* parent)
    : QWizardPage(parent),
      buttonsLayout(new QHBoxLayout),
      checkHeader(checkHeader),
      entities(entities),
      layout(new QVBoxLayout(this)),
      nameHeader(nameHeader),
      progressBar(new QProgressBar(this)),
      selectAllCheckBox(new QCheckBox("Select All", this)),
      startButton(new QPushButton("Start", this)),
      stopButton(new QPushButton("Stop", this)),
      table(new RichTableView(this)),
      threadPool(new QThreadPool(this))
{
    setSubTitle(subtitle);
    setTitle(title);

    progressBar->setValue(0);
    stopButton->setEnabled(false);
    threadPool->setMaxThreadCount(QThread::idealThreadCount() / 2);

    buttonsLayout->addWidget(selectAllCheckBox);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(stopButton);

    table->horizontalHeader()->setStretchLastSection(true);
    table->setItemDelegate(new RichItemDelegate(this));

    layout->addWidget(table);
    layout->addLayout(buttonsLayout);
    layout->addWidget(progressBar);

    connect(selectAllCheckBox, &QCheckBox::clicked, this, &ChooseEntitiesPage::selectAll);
    connect(startButton, &QPushButton::clicked, this, &ChooseEntitiesPage::startTask);
    connect(stopButton, &QPushButton::clicked, this, &ChooseEntitiesPage::stopTask);
    connect(this, &ChooseEntitiesPage::progress, this, &ChooseEntitiesPage::tickProgress);
}

ChooseEntitiesPage::~ChooseEntitiesPage()
{
    stopped = true;
}

void ChooseEntitiesPage::initializePage()
{
    EntitySelectTableModel* tableModel = new EntitySelectTableModel(checkHeader, nameHeader, this);
    for (const Entity& entity : std::as_const(entities))
        tableModel->append(entity);
    table->setModel(tableModel);
}

bool ChooseEntitiesPage::isComplete() const
{
    return progressBar->value() == progressBar->maximum();
}

void ChooseEntitiesPage::selectAll(bool checked)
{
    EntitySelectTableModel* tableModel = qobject_cast<EntitySelectTableModel*>(table->model());
    for (int i = 0; i < tableModel->rowCount(); i++)
        tableModel->setData(tableModel->index(i, 0), checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
    selectAllCheckBox->setText(checked ? "Deselect All" : "Select All");
}

void ChooseEntitiesPage::startTask()
{
    selectAllCheckBox->setEnabled(false);
    startButton->setEnabled(false);
    stopButton->setEnabled(true);

    EntitySelectTableModel* tableModel = qobject_cast<EntitySelectTableModel*>(table->model());
    progressBar->setMaximum(tableModel->checkedRowCount());
    for (int i = 0; i < tableModel->rowCount(); i++)
    {
        const Entity& entity = tableModel->entityAt(tableModel->index(i, 0));
        if (entity.checked)
            emit foundEntity(entity);
    }
}

void ChooseEntitiesPage::stopTask()
{
    stopped = true;
    stopButton->setEnabled(false);
}

void ChooseEntitiesPage::tickProgress()
{
    progressBar->setValue(!stopped ? progressBar->value() + 1 : progressBar->maximum());
    emit completeChanged();
}
