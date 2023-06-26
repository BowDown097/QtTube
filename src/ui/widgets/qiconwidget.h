#ifndef QICONWIDGET_H
#define QICONWIDGET_H
#include <QSvgWidget>

class QIconWidget : public QSvgWidget
{
    Q_OBJECT
public:
    explicit QIconWidget(const QString& iconId, const QSize& size = QSize(16, 16), QWidget* parent = nullptr);
    void setClickable(bool clickable) { this->clickable = clickable; }
    void setIcon(const QString& iconId, const QPalette& pal = QPalette());
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
    bool clickable = false;
};

#endif // QICONWIDGET_H
