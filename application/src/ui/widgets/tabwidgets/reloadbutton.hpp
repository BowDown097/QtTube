#pragma once
#include <QMovie>
#include <QPushButton>

class ReloadButton : public QPushButton
{
public:
    explicit ReloadButton(QWidget* parent = nullptr);
    QMovie* movie() const { return m_movie; }
    QSize sizeHint() const override;
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QMovie* m_movie{};
};
