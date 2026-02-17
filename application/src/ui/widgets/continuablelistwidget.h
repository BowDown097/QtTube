#pragma once
#include <any>
#include <QListWidget>

class ContinuableListWidget : public QListWidget
{
    Q_OBJECT
public:
    std::any continuationData;

    explicit ContinuableListWidget(QWidget* parent = nullptr);
    void setContinuationThreshold(int threshold) { m_continuationThreshold = threshold; }
    void toggleListGridLayout();

    bool isPopulating() const { return m_populating; }
    void setPopulatingFlag(bool populating) { m_populating = populating; }
protected:
    void updateGeometries() override;
    void wheelEvent(QWheelEvent* event) override;
private:
    int m_continuationThreshold = 10;
    bool m_populating{};
public slots:
    void clear();
private slots:
    void scrollValueChanged(int value);
signals:
    void continuationReady();
};
