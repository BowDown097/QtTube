#pragma once
#include <QLabel>

class ElidedTubeLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedTubeLabel(QWidget* parent = nullptr) : QLabel(parent) {}
    ElidedTubeLabel(const QString& text, QWidget* parent = nullptr) : ElidedTubeLabel(parent) { setText(text); }

    void setClickable(bool clickable, bool underline) { m_clickable = clickable; m_underline = underline; }
    void setText(const QString& text);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
signals:
    void clicked();
private:
    bool m_clickable = false;
    bool m_underline = false;
};
