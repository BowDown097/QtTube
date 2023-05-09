QT += core gui network widgets
INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src
CONFIG += c++2a

win32: LIBS += -luser32

include(lib/http/http.pri)
include(lib/innertube-qt/innertube-qt.pri)

unix:!macx {
    load(configure)
    qtCompileTest(xscreensaver)
    CONFIG(config_xscreensaver) {
        qtHaveModule(x11extras) {
            DEFINES += QTTUBE_HAS_X11EXTRAS
            QT += x11extras
        }
        DEFINES += QTTUBE_HAS_XSS
        LIBS += -lX11 -lXss
    }
}

contains(DEFINES, USEMPV) {
    LIBS += -lmpv
    QT += openglwidgets

    SOURCES += \
        lib/media/mpv/mediampv.cpp \
        lib/media/mpv/mpvwidget.cpp

    HEADERS += \
        lib/media/media.h \
        lib/media/mpv/mediampv.h \
        lib/media/mpv/mpvwidget.h \
        lib/media/mpv/qthelper.hpp
}

!contains(DEFINES, USEMPV) {
    QT += webenginewidgets

    SOURCES += \
        src/ui/widgets/webengineplayer/fullscreennotification.cpp \
        src/ui/widgets/webengineplayer/fullscreenwindow.cpp \
        src/ui/widgets/webengineplayer/playerinterceptor.cpp \
        src/ui/widgets/webengineplayer/webchannelinterface.cpp \
        src/ui/widgets/webengineplayer/webengineplayer.cpp

    HEADERS += \
        src/ui/widgets/webengineplayer/fullscreennotification.h \
        src/ui/widgets/webengineplayer/fullscreenwindow.h \
        src/ui/widgets/webengineplayer/playerinterceptor.h \
        src/ui/widgets/webengineplayer/webchannelinterface.h \
        src/ui/widgets/webengineplayer/webengineplayer.h
}

SOURCES += \
    src/main.cpp \
    src/osutilities.cpp \
    src/qttubeapplication.cpp \
    src/settingsstore.cpp \
    src/ui/browsehelper.cpp \
    src/ui/channelbrowser.cpp \
    src/ui/forms/livechatwindow.cpp \
    src/ui/uiutilities.cpp \
    src/ui/forms/mainwindow.cpp \
    src/ui/forms/settingsform.cpp \
    src/ui/views/channelview.cpp \
    src/ui/views/viewcontroller.cpp \
    src/ui/views/watchview.cpp \
    src/ui/views/watchview_ui.cpp \
    src/ui/views/watchviewplayer.cpp \
    src/ui/widgets/labels/channellabel.cpp \
    src/ui/widgets/labels/iconlabel.cpp \
    src/ui/widgets/labels/tubelabel.cpp \
    src/ui/widgets/renderers/browsechannelrenderer.cpp \
    src/ui/widgets/renderers/browsenotificationrenderer.cpp \
    src/ui/widgets/renderers/browsevideorenderer.cpp \
    src/ui/widgets/findbar.cpp \
    src/ui/widgets/subscribe/notificationbell.cpp \
    src/ui/widgets/subscribe/subscribelabel.cpp \
    src/ui/widgets/subscribe/subscribewidget.cpp \
    src/ui/widgets/topbar.cpp

HEADERS += \
    src/osutilities.h \
    src/qttubeapplication.h \
    src/settingsstore.h \
    src/ui/browsehelper.h \
    src/ui/browsehelper.tpp \
    src/ui/channelbrowser.h \
    src/ui/forms/livechatwindow.h \
    src/ui/uiutilities.h \
    src/ui/uiutilities.tpp \
    src/ui/forms/mainwindow.h \
    src/ui/forms/settingsform.h \
    src/ui/views/channelview.h \
    src/ui/views/viewcontroller.h \
    src/ui/views/watchview.h \
    src/ui/views/watchview_ui.h \
    src/ui/views/watchviewplayer.h \
    src/ui/widgets/labels/channellabel.h \
    src/ui/widgets/labels/iconlabel.h \
    src/ui/widgets/labels/tubelabel.h \
    src/ui/widgets/renderers/browsechannelrenderer.h \
    src/ui/widgets/renderers/browsenotificationrenderer.h \
    src/ui/widgets/renderers/browsevideorenderer.h \
    src/ui/widgets/findbar.h \
    src/ui/widgets/subscribe/notificationbell.h \
    src/ui/widgets/subscribe/subscribelabel.h \
    src/ui/widgets/subscribe/subscribewidget.h \
    src/ui/widgets/topbar.h

FORMS += \
    src/ui/forms/livechatwindow.ui \
    src/ui/forms/mainwindow.ui \
    src/ui/forms/settingsform.ui

RESOURCES += res/resources.qrc
win32: RC_ICONS = res/qttube.ico
macx: ICON = res/qttube.icns

QTQUICK_COMPILER_SKIPPED_RESOURCES += res/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target
