QT       += core gui network webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    innertube/auth.hpp \
    innertube/objects/InnertubeClickTracking.h \
    innertube/objects/InnertubeClient.h \
    innertube/objects/InnertubeConfigInfo.h \
    innertube/objects/InnertubeContext.h \
    innertube/objects/InnertubeRequestConfig.h \
    innertube/objects/InnertubeUserConfig.h \
    innertube/requests.hpp \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
