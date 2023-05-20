#ifndef TOPBARBELL_H
#define TOPBARBELL_H
#include <QLabel>

class TopBarBell : public QWidget
{
    Q_OBJECT
public:
    QLabel* bell;
    QLabel* count;

    explicit TopBarBell(QWidget* parent = nullptr);
    void updatePixmap(bool hasNotif, bool preferDark);
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
};

#endif // TOPBARBELL_H
