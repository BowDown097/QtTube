#pragma once
#include <QLabel>

class FullScreenNotification : public QLabel
{
    Q_OBJECT
public:
    explicit FullScreenNotification(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
signals:
    void shown();
private:
    bool m_previouslyVisible;
};
