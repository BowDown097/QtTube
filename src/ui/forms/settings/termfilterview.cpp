#include "termfilterview.h"
#include "ui_termfilterview.h"
#include "qttubeapplication.h"
#include <QMessageBox>

TermFilterView::TermFilterView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TermFilterView)
{
    ui->setupUi(this);
    connect(ui->addButton, &QPushButton::clicked, this, &TermFilterView::addNewRow);
    connect(ui->listWidget, &QListWidget::itemChanged, this, &TermFilterView::registerInSettings);
    connect(ui->removeButton, &QPushButton::clicked, this, &TermFilterView::removeCurrentRow);
}

void TermFilterView::addNewRow()
{
    QListWidgetItem* item = new QListWidgetItem;
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    ui->listWidget->addItem(item);
}

void TermFilterView::populateFromSettings()
{
    for (const QString& term : qtTubeApp->settings().filteredTerms)
    {
        QListWidgetItem* item = new QListWidgetItem(term);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    }
}

void TermFilterView::registerInSettings(QListWidgetItem* item)
{
    if (item->text().isEmpty())
        return;

    if (ui->listWidget->findItems(item->text(), Qt::MatchFixedString).count() > 1)
    {
        ui->listWidget->removeItemWidget(item);
        QMessageBox::critical(this, "Duplicate term", "There is already a filter for this term.");
        return;
    }

    if (!qtTubeApp->settings().filteredTerms.contains(item->text()))
        qtTubeApp->settings().filteredTerms.append(item->text());
}

void TermFilterView::removeCurrentRow()
{
    QItemSelectionModel* selModel = ui->listWidget->selectionModel();
    if (!selModel->hasSelection())
        return;

    int row = selModel->selectedRows().constFirst().row();
    QListWidgetItem* item = ui->listWidget->item(row);
    qtTubeApp->settings().filteredTerms.removeOne(item->text());
    delete item;
}

TermFilterView::~TermFilterView()
{
    delete ui;
}
