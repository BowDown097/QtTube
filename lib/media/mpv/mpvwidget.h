#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QOpenGLWidget>

#include <mpv/client.h>
#include <mpv/render_gl.h>

class MpvWidget Q_DECL_FINAL : public QOpenGLWidget {
    Q_OBJECT

public:
    MpvWidget(mpv_handle *mpv, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Widget);
    ~MpvWidget() Q_DECL_OVERRIDE;

    QSize sizeHint() const Q_DECL_OVERRIDE { return QSize(480, 270); }

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private slots:
    void maybeUpdate();
    void onFrameSwapped();

private:
    static void onUpdate(void *ctx);

    mpv_handle *mpv;
    mpv_render_context *mpvContext;
    int glWidth;
    int glHeight;
};

#endif // PLAYERWINDOW_H
