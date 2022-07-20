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
    include/innertube/objects/InnertubeClickTracking.h \
    include/innertube/objects/InnertubeClient.h \
    include/innertube/objects/InnertubeConfigInfo.h \
    include/innertube/objects/InnertubeContext.h \
    include/innertube/objects/InnertubeRequestConfig.h \
    include/innertube/objects/InnertubeUserConfig.h \
    include/innertube/requests.hpp \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
