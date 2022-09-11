#include "mpvwidget.h"
#include <stdexcept>

#include <QOpenGLContext>
#include <QtCore>

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
#include <QGuiApplication>
#endif

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

MpvWidget::MpvWidget(mpv_handle *mpv, QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f), mpv(mpv), mpvContext(nullptr) {
    moveToThread(qApp->thread());
}

MpvWidget::~MpvWidget() {
    makeCurrent();
    if (mpvContext) mpv_render_context_free(mpvContext);
    mpv_terminate_destroy(mpv);
}

void MpvWidget::initializeGL() {
    if (mpvContext) qFatal("Already initialized");

    QWidget *nativeParent = nativeParentWidget();
    qDebug() << "initializeGL" << nativeParent;
    if (nativeParent == nullptr) qFatal("No native parent");

    mpv_opengl_init_params gl_init_params{get_proc_address, this, nullptr};
    mpv_render_param params[]{{MPV_RENDER_PARAM_API_TYPE, (void *)MPV_RENDER_API_TYPE_OPENGL},
                              {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                              {MPV_RENDER_PARAM_INVALID, nullptr},
                              {MPV_RENDER_PARAM_INVALID, nullptr}};

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
    const QString platformName = QGuiApplication::platformName();
    if (platformName.contains("xcb")) {
        params[2].type = MPV_RENDER_PARAM_X11_DISPLAY;
        params[2].data = (void *)qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
        qDebug() << platformName << params[2].data;
    } else if (platformName.contains("wayland")) {
        qWarning() << "Wayland not supported";
    }
#endif

    if (mpv_render_context_create(&mpvContext, mpv, params) < 0)
        qFatal("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpvContext, MpvWidget::onUpdate, (void *)this);

    connect(this, &QOpenGLWidget::frameSwapped, this, &MpvWidget::onFrameSwapped);
}

void MpvWidget::resizeGL(int w, int h) {
    qreal r = devicePixelRatioF();
    glWidth = int(w * r);
    glHeight = int(h * r);
}

void MpvWidget::paintGL() {
    mpv_opengl_fbo fbo{static_cast<int>(defaultFramebufferObject()), glWidth, glHeight, 0};

    static bool yes = true;
    mpv_render_param params[] = {{MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
                                 {MPV_RENDER_PARAM_FLIP_Y, &yes},
                                 {MPV_RENDER_PARAM_INVALID, nullptr}};
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(mpvContext, params);
}

// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void MpvWidget::maybeUpdate() {
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's opengl-cb API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (!updatesEnabled() || isHidden() || window()->isHidden() || window()->isMinimized()) {
        makeCurrent();
        paintGL();
        QOpenGLContext *c = context();
        c->swapBuffers(c->surface());
        doneCurrent();
        mpv_render_context_report_swap(mpvContext);
    } else {
        update();
    }
}

void MpvWidget::onFrameSwapped() {
    mpv_render_context_report_swap(mpvContext);
}

void MpvWidget::onUpdate(void *ctx) {
    QMetaObject::invokeMethod((MpvWidget *)ctx, "maybeUpdate");
}
