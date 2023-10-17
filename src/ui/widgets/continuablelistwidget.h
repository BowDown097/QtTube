#ifndef CONTINUABLELISTWIDGET_H
#define CONTINUABLELISTWIDGET_H
#include <QListWidget>

class ContinuableListWidget : public QListWidget
{
    Q_OBJECT
public:
    bool continuationRunning = false;
    QString continuationToken;

    explicit ContinuableListWidget(QWidget* parent = nullptr);
    void setContinuationThreshold(int threshold) { continuationThreshold = threshold; }
private:
    int continuationThreshold = 10;
private slots:
    void scrollValueChanged(int value);
signals:
    void continuationReady();
};

#endif // CONTINUABLELISTWIDGET_H
