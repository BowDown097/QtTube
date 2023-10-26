#include "channelfiltertable.h"
#include "ui_channelfiltertable.h"
#include "innertube.h"
#include "qttubeapplication.h"
#include <QMessageBox>

ChannelFilterTable::ChannelFilterTable(QWidget* parent) : QWidget(parent), ui(new Ui::ChannelFilterTable)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->addButton, &QPushButton::clicked, this, &ChannelFilterTable::addNewRow);
    connect(ui->removeButton, &QPushButton::clicked, this, &ChannelFilterTable::removeCurrentRow);
    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &ChannelFilterTable::validateItemInput);
}

void ChannelFilterTable::addNewRow()
{
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    QTableWidgetItem* item = new QTableWidgetItem;
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, item);
}

void ChannelFilterTable::populateFromSettings()
{
    populating = true;
    for (const QString& channelId : qtTubeApp->settings().filteredChannels)
    {
        addNewRow();
        QTableWidgetItem* item = new QTableWidgetItem(channelId);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, item);
    }
    populating = false;

    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        validateItemInput(ui->tableWidget->item(i, 0));
}

void ChannelFilterTable::processChannelEntry(const InnertubeEndpoints::BrowseChannel& channel, QTableWidgetItem* item)
{
    QString channelHandle = channel.response.header.channelHandleText.text;
    QString channelId = channel.response.header.channelId;

    if (channelHandle.isEmpty())
    {
        ui->tableWidget->removeRow(item->row());
        QMessageBox::critical(this, "Invalid channel ID", "Could not find a channel with that ID.");
        return;
    }

    QTableWidgetItem* handleItem = ui->tableWidget->item(item->row(), 1);
    handleItem->setText(channelHandle);

    if (!qtTubeApp->settings().filteredChannels.contains(channelId))
        qtTubeApp->settings().filteredChannels.append(channelId);
}

void ChannelFilterTable::removeCurrentRow()
{
    QItemSelectionModel* selModel = ui->tableWidget->selectionModel();
    if (!selModel->hasSelection())
        return;

    int row = selModel->selectedRows().constFirst().row();
    qtTubeApp->settings().filteredChannels.removeOne(ui->tableWidget->item(row, 0)->text());
    ui->tableWidget->removeRow(row);
}

void ChannelFilterTable::validateItemInput(QTableWidgetItem* item)
{
    if (item->column() == 1 || populating || item->text().isEmpty())
        return;

    if (ui->tableWidget->findItems(item->text(), Qt::MatchFixedString).count() > 1)
    {
        ui->tableWidget->removeRow(item->row());
        QMessageBox::critical(this, "Duplicate channel ID", "There is already a filter for this channel ID.");
        return;
    }

    processChannelEntry(InnerTube::instance().getBlocking<InnertubeEndpoints::BrowseChannel>(item->text()), item);
}

ChannelFilterTable::~ChannelFilterTable()
{
    delete ui;
}
