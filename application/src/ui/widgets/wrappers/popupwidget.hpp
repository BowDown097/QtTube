#pragma once
#include <QFrame>
#include <QPointer>

class PopupWidget : public QFrame
{
    Q_OBJECT
public:
    PopupWidget(QWidget* base, QWidget* anchor);
private:
    QPointer<QWidget> m_anchor;
    QPointer<QWidget> m_base;

    QRect baseGeometry() const;
    QRect desiredOpenGeometry() const;
public slots:
    void showPopup();
};
