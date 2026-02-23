#pragma once
#include "ui/widgets/wrappers/clickablewidget.h"

class QHBoxLayout;
class QLabel;

class IconLabel : public ClickableWidget<>
{
    Q_OBJECT
public:
    explicit IconLabel(
        const QString& iconId, const QMargins& contentsMargins = QMargins(),
        const QSize& size = QSize(16, 16), QWidget* parent = nullptr);
    explicit IconLabel(
        const QString& iconId, const QString& text, const QMargins& contentsMargins = QMargins(),
        const QSize& size = QSize(16, 16), QWidget* parent = nullptr);

    void setIcon(const QString& iconId);
    void setStyleSheet(const QString& styleSheet);
    void setText(const QString& text);

    QString styleSheet() const;
    QString text() const;
protected:
    void changeEvent(QEvent* event) override;
private:
    QLabel* m_icon;
    QString m_iconId;
    QHBoxLayout* m_layout;
    QLabel* m_text;
};
