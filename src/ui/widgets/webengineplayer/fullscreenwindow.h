#ifndef FULLSCREENWINDOW_H
#define FULLSCREENWINDOW_H
#include "fullscreennotification.h"
#include <QWebEngineView>
#include <QWidget>

class FullScreenWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FullScreenWindow(QWebEngineView* oldView, QWidget* parent = nullptr);
    ~FullScreenWindow();
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    FullScreenNotification* m_notification;
    QRect m_oldGeometry;
    QWebEngineView* m_oldView;
    QWebEngineView* m_view;
};

#endif // FULLSCREENWINDOW_H
