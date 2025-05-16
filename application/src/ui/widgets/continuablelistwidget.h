#pragma once
#include <QListWidget>

class ContinuableListWidget : public QListWidget
{
    Q_OBJECT
public:
    QString continuationToken;

    explicit ContinuableListWidget(QWidget* parent = nullptr);
    void setContinuationThreshold(int threshold) { continuationThreshold = threshold; }
    void toggleListGridLayout();

    bool isPopulating() const { return populating; }
    void setPopulatingFlag(bool populating) { this->populating = populating; }
protected:
    void updateGeometries() override;
    void wheelEvent(QWheelEvent* event) override;
private:
    int continuationThreshold = 10;
    bool populating{};
public slots:
    void clear();
private slots:
    void scrollValueChanged(int value);
signals:
    void continuationReady();
};
