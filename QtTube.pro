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
        ui/watchview-mpv.cpp
    HEADERS += \
        lib/media/media.h \
        lib/media/mpv/mediampv.h \
        lib/media/mpv/mpvwidget.h \
        lib/media/mpv/qthelper.hpp \
        ui/watchview-mpv.h
}

!contains(DEFINES, USEMPV) {
    include(lib/webengineplayer/WebEnginePlayer.pri)
    SOURCES += ui/watchview-ytp.cpp
    HEADERS += ui/watchview-ytp.h
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ui/settingsform.cpp \
    ui/topbar.cpp

HEADERS += \
    browsehelper.hpp \
    mainwindow.h \
    qttubeapplication.hpp \
    settingsstore.hpp \
    ui/browsechannelrenderer.h \
    ui/browsevideorenderer.h \
    ui/clickablelabel.h \
    ui/settingsform.h \
    ui/topbar.h \
    ui/watchview-shared.h

FORMS += \
    mainwindow.ui \
    ui/settingsform.ui

RESOURCES += res/resources.qrc
win32: RC_ICONS = res/qttube.ico
macx: ICON = res/qttube.icns

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
