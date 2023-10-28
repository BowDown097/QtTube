#ifndef CHANNELFILTERTABLE_H
#define CHANNELFILTERTABLE_H
#include "innertube/endpoints/browse/browsechannel.h"
#include <QTableWidgetItem>

namespace Ui {
class ChannelFilterTable;
}

class ChannelFilterTable : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelFilterTable(QWidget* parent = nullptr);
    ~ChannelFilterTable();
    void populateFromSettings();
private:
    bool populating;
    Ui::ChannelFilterTable *ui;
    void processChannelEntry(const InnertubeEndpoints::BrowseChannel& channel, QTableWidgetItem* item);
private slots:
    void addNewRow();
    void removeCurrentRow();
    void validateItemInput(QTableWidgetItem* item);
};

#endif // CHANNELFILTERTABLE_H
