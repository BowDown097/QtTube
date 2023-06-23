#ifndef TUBELABEL_H
#define TUBELABEL_H
#include "innertube/objects/innertubestring.h"
#include <QLabel>

class TubeLabel : public QLabel
{
    Q_OBJECT
public:
    explicit TubeLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit TubeLabel(const InnertubeObjects::InnertubeString& itStr, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit TubeLabel(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setClickable(bool clickable, bool underline) { this->clickable = clickable; this->underline = underline; }
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
    bool underline = false;
};

#endif // TUBELABEL_H
