#pragma once
#include <QLabel>

class QHBoxLayout;

class IconLabel : public QWidget
{
    Q_OBJECT
public:
    QLabel* icon;
    QLabel* textLabel;

    explicit IconLabel(const QString& iconId, const QMargins& contentsMargins = QMargins(), const QSize& size = QSize(16, 16),
                       QWidget* parent = nullptr);
    IconLabel(const QString& iconId, const QString& text, const QMargins& contentsMargins = QMargins(),
              const QSize& size = QSize(16, 16), QWidget* parent = nullptr);

    void setIcon(const QString& iconId);

    void setStyleSheet(const QString& styleSheet) { textLabel->setStyleSheet(styleSheet); }
    QString styleSheet() const { return textLabel->styleSheet(); }

    void setText(const QString& text) { textLabel->setText(text); }
    QString text() const { return textLabel->text(); }
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
