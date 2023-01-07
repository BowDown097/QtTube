QT += core gui network webenginewidgets
INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++20
unix:!macx: LIBS += -lX11 -lXss

include(lib/http/http.pri)
include(lib/innertube-qt/innertube-qt.pri)

contains(DEFINES, USEMPV) {
    LIBS += -lmpv
    QT += openglwidgets
    lessThan(QT_MAJOR_VERSION, 6): QT += x11extras
    SOURCES += \
        lib/media/mpv/mediampv.cpp \
        lib/media/mpv/mpvwidget.cpp \
        src/ui/views/watchview-mpv.cpp
    HEADERS += \
        lib/media/media.h \
        lib/media/mpv/mediampv.h \
        lib/media/mpv/mpvwidget.h \
        lib/media/mpv/qthelper.hpp \
        src/ui/views/watchview-mpv.h
}

!contains(DEFINES, USEMPV) {
    include(lib/webengineplayer/WebEnginePlayer.pri)
    SOURCES += src/ui/views/watchview-ytp.cpp
    HEADERS += src/ui/views/watchview-ytp.h
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/browsehelper.cpp \
    src/main.cpp \
    src/qttubeapplication.cpp \
    src/settingsstore.cpp \
    src/ui/uiutilities.cpp \
    src/ui/forms/mainwindow.cpp \
    src/ui/forms/settingsform.cpp \
    src/ui/views/channelview.cpp \
    src/ui/views/watchview-shared.cpp \
    src/ui/widgets/browsechannelrenderer.cpp \
    src/ui/widgets/browsenotificationrenderer.cpp \
    src/ui/widgets/browsevideorenderer.cpp \
    src/ui/widgets/subscribewidget.cpp \
    src/ui/widgets/topbar.cpp \
    src/ui/widgets/tubelabel.cpp

HEADERS += \
    src/browsehelper.h \
    src/browsehelper.tpp \
    src/qttubeapplication.h \
    src/settingsstore.h \
    src/ui/uiutilities.h \
    src/ui/forms/mainwindow.h \
    src/ui/forms/settingsform.h \
    src/ui/views/channelview.h \
    src/ui/views/watchview-shared.h \
    src/ui/widgets/browsechannelrenderer.h \
    src/ui/widgets/browsenotificationrenderer.h \
    src/ui/widgets/browsevideorenderer.h \
    src/ui/widgets/subscribewidget.h \
    src/ui/widgets/topbar.h \
    src/ui/widgets/tubelabel.h

FORMS += \
    src/ui/forms/mainwindow.ui \
    src/ui/forms/settingsform.ui

RESOURCES += res/resources.qrc
win32: RC_ICONS = res/qttube.ico
macx: ICON = res/qttube.icns

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
