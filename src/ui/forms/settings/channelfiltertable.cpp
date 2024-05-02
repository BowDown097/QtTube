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
    for (const QString& channel : qtTubeApp->settings().filteredChannels)
    {
        QStringList split = channel.split('|');
        addNewRow();

        QTableWidgetItem* channelIdItem = new QTableWidgetItem(split[0]);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, channelIdItem);

        if (split.size() > 1)
        {
            QTableWidgetItem* channelHandleItem = new QTableWidgetItem(split[1]);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, channelHandleItem);
        }
    }
    populating = false;

    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        if (ui->tableWidget->item(i, 1)->text().isEmpty())
            validateItemInput(ui->tableWidget->item(i, 0));
}

void ChannelFilterTable::processChannelEntry(const InnertubeEndpoints::BrowseChannel& channel, QTableWidgetItem* item)
{
    QString channelHandle = channel.response.header.metadata.metadataRows[0][0];
    QString channelId = channel.response.metadata.externalId;

    if (channelHandle.isEmpty())
    {
        QMessageBox::critical(this, "Invalid channel ID", "Could not find a channel with the ID \"" + item->text() + "\".");
        ui->tableWidget->removeRow(item->row());
        return;
    }

    QStringList& filteredChannels = qtTubeApp->settings().filteredChannels;
    auto matchingIt = std::ranges::find_if(filteredChannels, [&channelId](const QString& s) {
        return s.startsWith(channelId);
    });

    if (matchingIt != filteredChannels.end())
    {
        if (QStringList split = matchingIt->split('|'); split.size() > 1 && split[1] == channelHandle)
            return;
        filteredChannels.removeOne(*matchingIt);
    }

    QTableWidgetItem* handleItem = ui->tableWidget->item(item->row(), 1);
    handleItem->setText(channelHandle);

    filteredChannels.append(channelId + "|" + channelHandle);
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
    if (!item || item->column() == 1 || populating || item->text().isEmpty())
        return;

    if (ui->tableWidget->findItems(item->text(), Qt::MatchFixedString).count() > 1)
    {
        QMessageBox::critical(this, "Duplicate channel ID", "There is already a filter for the channel ID \"" + item->text() + "\".");
        ui->tableWidget->removeRow(item->row());
        return;
    }

    processChannelEntry(InnerTube::instance()->getBlocking<InnertubeEndpoints::BrowseChannel>(item->text()), item);
}

ChannelFilterTable::~ChannelFilterTable()
{
    delete ui;
}
