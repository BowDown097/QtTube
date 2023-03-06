#ifndef ICONLABEL_H
#define ICONLABEL_H
#include <QHBoxLayout>
#include <QLabel>

class IconLabel : public QWidget
{
    Q_OBJECT
public:
    QLabel* icon;
    QLabel* textLabel;
    IconLabel(const QString& iconId, const QMargins& contentsMargins = QMargins(0, 0, 0, 0), QWidget* parent = nullptr);
    void setText(const QString& text) { textLabel->setText(text); }
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
