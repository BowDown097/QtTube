#ifndef TERMFILTERVIEW_H
#define TERMFILTERVIEW_H
#include <QWidget>

namespace Ui {
class TermFilterView;
}

class QListWidgetItem;

class TermFilterView : public QWidget
{
    Q_OBJECT
public:
    explicit TermFilterView(QWidget* parent = nullptr);
    ~TermFilterView();

    void populateFromSettings();
private:
    Ui::TermFilterView *ui;
private slots:
    void addNewRow();
    void registerInSettings(QListWidgetItem* item);
    void removeCurrentRow();
};

#endif // TERMFILTERVIEW_H
