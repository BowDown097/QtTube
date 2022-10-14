QT += core gui network webenginewidgets
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
        src/ui/watchview-mpv.cpp
    HEADERS += \
        lib/media/media.h \
        lib/media/mpv/mediampv.h \
        lib/media/mpv/mpvwidget.h \
        lib/media/mpv/qthelper.hpp \
        src/ui/watchview-mpv.h
}

!contains(DEFINES, USEMPV) {
    include(lib/webengineplayer/WebEnginePlayer.pri)
    SOURCES += src/ui/watchview-ytp.cpp
    HEADERS += src/ui/watchview-ytp.h
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/browsehelper.cpp \
    src/main.cpp \
    src/qttubeapplication.cpp \
    src/settingsstore.cpp \
    src/ui/browsechannelrenderer.cpp \
    src/ui/browsevideorenderer.cpp \
    src/ui/clickablelabel.cpp \
    src/ui/mainwindow.cpp \
    src/ui/settingsform.cpp \
    src/ui/topbar.cpp

HEADERS += \
    src/browsehelper.h \
    src/browsehelper.tpp \
    src/qttubeapplication.h \
    src/settingsstore.h \
    src/ui/clickablelabel.h \
    src/ui/mainwindow.h \
    src/ui/browsechannelrenderer.h \
    src/ui/browsevideorenderer.h \
    src/ui/settingsform.h \
    src/ui/topbar.h \
    src/ui/watchview-shared.hpp

FORMS += \
    src/ui/mainwindow.ui \
    src/ui/settingsform.ui

RESOURCES += res/resources.qrc
win32: RC_ICONS = res/qttube.ico
macx: ICON = res/qttube.icns

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
