#ifndef ICONLABEL_H
#define ICONLABEL_H
#include <QHBoxLayout>
#include <QLabel>
#include "ui/widgets/qiconwidget.h"

class IconLabel : public QWidget
{
    Q_OBJECT
public:
    QIconWidget* icon;
    QLabel* textLabel;
    explicit IconLabel(const QString& iconId, const QMargins& contentsMargins = QMargins(), const QSize& size = QSize(16, 16),
                       QWidget* parent = nullptr);
    IconLabel(const QString& iconId, const QString& text, const QMargins& contentsMargins = QMargins(),
              const QSize& size = QSize(16, 16), QWidget* parent = nullptr);
    void setIcon(const QString& iconId);
    void setText(const QString& text);
signals:
    void clicked();
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
private:
    QHBoxLayout* layout;
};

#endif // ICONLABEL_H
