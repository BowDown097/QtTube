#pragma once
#include <QTableView>

// modified version of https://stackoverflow.com/a/35418703
class RichTableView : public QTableView
{
public:
    explicit RichTableView(QWidget* parent = nullptr) : QTableView(parent) { setMouseTracking(true); }
protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private:
    QString lastHoveredAnchor;
    QString mousePressAnchor;
    QString anchorAt(const QPoint& point) const;
};
