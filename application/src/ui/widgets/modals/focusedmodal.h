#pragma once
#include <QWidget>

class QPropertyAnimation;

class FocusedModal : public QWidget
{
public:
    explicit FocusedModal(QWidget* parent);
    void reorient();
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    // has to be a child of parent for layering to work, so we can't rely on the standard ownership model for deletion
    QScopedPointer<QWidget, QScopedPointerDeleteLater> backdrop;

    QPropertyAnimation* animationFor(QWidget* widget);
};
