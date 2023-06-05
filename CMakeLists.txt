cmake_minimum_required(VERSION 3.16)
project(QtTube VERSION 1.0 LANGUAGES C CXX)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC, AUTORCC, and AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
include(GNUInstallDirs)

# Compiler flags
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# Application options
set(QTTUBE_APP_DESC "A Qt frontend for YouTube." CACHE STRING "The description to use for the application.")
set(QTTUBE_APP_NAME "QtTube" CACHE STRING "The name to use for the application.")
set(QTTUBE_REPO_URL "https://github.com/BowDown097/QtTube" CACHE STRING "Git repository URL to be displayed in the about dialog.")
set(QTTUBE_VERSION_NAME "ALPHA")

# Compilation options
option(QTTUBE_USEMPV "Use MPV for the video player." OFF)

# Qt packages
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)

if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS Gui Network Widgets)
    find_package(Qt5 OPTIONAL_COMPONENTS OpenGL WebEngineWidgets X11Extras)
elseif(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Gui Network Widgets)
    find_package(Qt6 OPTIONAL_COMPONENTS OpenGLWidgets WebEngineWidgets)
else()
    message(FATAL_ERROR "Qt version ${QT_VERSION_MAJOR} is not supported")
endif()

# macOS bundle settings
if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "${QTTUBE_APP_NAME}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${QTTUBE_VERSION_NAME}")
    set(MACOSX_BUNDLE_INFO_STRING "${QTTUBE_APP_NAME}: ${QTTUBE_APP_DESC}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${QTTUBE_VERSION_NAME}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${QTTUBE_VERSION_NAME}")
endif()

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/osutilities.cpp
    src/qttubeapplication.cpp
    src/settingsstore.cpp
    src/ui/browsehelper.cpp
    src/ui/channelbrowser.cpp
    src/ui/uiutilities.cpp
    src/ui/forms/livechatwindow.cpp
    src/ui/forms/mainwindow.cpp
    src/ui/forms/settingsform.cpp
    src/ui/views/channelview.cpp
    src/ui/views/viewcontroller.cpp
    src/ui/views/watchview.cpp
    src/ui/views/watchview_ui.cpp
    src/ui/views/watchviewplayer.cpp
    src/ui/widgets/findbar.cpp
    src/ui/widgets/labels/channellabel.cpp
    src/ui/widgets/labels/iconlabel.cpp
    src/ui/widgets/labels/tubelabel.cpp
    src/ui/widgets/renderers/browsechannelrenderer.cpp
    src/ui/widgets/renderers/browsenotificationrenderer.cpp
    src/ui/widgets/renderers/browsevideorenderer.cpp
    src/ui/widgets/subscribe/notificationbell.cpp
    src/ui/widgets/subscribe/subscribelabel.cpp
    src/ui/widgets/subscribe/subscribewidget.cpp
    src/ui/widgets/topbar/topbar.cpp
    src/ui/widgets/topbar/topbarbell.cpp
    res/resources.qrc
)

set(HEADERS
    src/osutilities.h
    src/qttubeapplication.h
    src/settingsstore.h
    src/ui/browsehelper.h
    src/ui/browsehelper.tpp
    src/ui/channelbrowser.h
    src/ui/uiutilities.h
    src/ui/uiutilities.tpp
    src/ui/forms/livechatwindow.h
    src/ui/forms/mainwindow.h
    src/ui/forms/settingsform.h
    src/ui/views/channelview.h
    src/ui/views/viewcontroller.h
    src/ui/views/watchview.h
    src/ui/views/watchview_ui.h
    src/ui/views/watchviewplayer.h
    src/ui/widgets/findbar.h
    src/ui/widgets/labels/channellabel.h
    src/ui/widgets/labels/iconlabel.h
    src/ui/widgets/labels/tubelabel.h
    src/ui/widgets/renderers/browsechannelrenderer.h
    src/ui/widgets/renderers/browsenotificationrenderer.h
    src/ui/widgets/renderers/browsevideorenderer.h
    src/ui/widgets/subscribe/notificationbell.h
    src/ui/widgets/subscribe/subscribelabel.h
    src/ui/widgets/subscribe/subscribewidget.h
    src/ui/widgets/topbar/topbar.h
    src/ui/widgets/topbar/topbarbell.h
)

set(FORMS
    src/ui/forms/livechatwindow.ui
    src/ui/forms/mainwindow.ui
    src/ui/forms/settingsform.ui
)

# Application icon
if(WIN32 OR APPLE)
    include(${CMAKE_SOURCE_DIR}/cmake/AddIconToBinary.cmake)
    AddIconToBinary(SOURCE_FILES ICONS ${CMAKE_SOURCE_DIR}/res/qttube.ico ${CMAKE_SOURCE_DIR}/res/qttube.icns)
endif()

# Add executable
add_subdirectory(lib/http) # TODO: ditch http library
add_subdirectory(lib/innertube-qt)
if(WIN32)
    add_executable(QtTube WIN32 ${SOURCE_FILES} ${HEADERS} ${FORMS})
elseif(APPLE)
    add_executable(QtTube MACOSX_BUNDLE ${SOURCE_FILES} ${HEADERS} ${FORMS})
else()
    add_executable(QtTube ${SOURCE_FILES} ${HEADERS} ${FORMS})
endif()

# Define variables and Git details in preprocessor
execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE QTTUBE_BRANCH)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE QTTUBE_COMMIT_ID)
target_compile_definitions(QtTube PRIVATE
    QTTUBE_APP_DESC="${QTTUBE_APP_DESC}"
    QTTUBE_APP_NAME="${QTTUBE_APP_NAME}"
    QTTUBE_REPO_URL="${QTTUBE_REPO_URL}"
    QTTUBE_VERSION_NAME="${QTTUBE_VERSION_NAME}"
    QTTUBE_BRANCH="${QTTUBE_BRANCH}"
    QTTUBE_COMMIT_ID="${QTTUBE_COMMIT_ID}"
)

# Include directories, libraries
target_include_directories(QtTube PRIVATE src lib/http/src lib/innertube-qt/src)
target_link_libraries(QtTube PRIVATE cpr::cpr http innertube-qt Qt::Core Qt::Gui Qt::Network Qt::Widgets)

## Optional
if(QTTUBE_USEMPV)
    target_sources(QtTube PRIVATE
        lib/media/media.h
        lib/media/mpv/mediampv.cpp lib/media/mpv/mediampv.h
        lib/media/mpv/mpvwidget.cpp lib/media/mpv/mpvwidget.h
        lib/media/mpv/qthelper.hpp
    )
    target_link_libraries(QtTube PRIVATE Qt::OpenGLWidgets)

    find_package(Libmpv)
    if(Libmpv_FOUND)
        target_include_directories(QtTube PRIVATE ${Libmpv_INCLUDE_DIRS})
        target_link_libraries(QtTube PRIVATE ${Libmpv_LIBRARIES})
        message(STATUS "Found libmpv ${Libmpv_VERSION}")
    else()
        message(FATAL_ERROR "Libmpv not found... is it installed?")
    endif()
else()
    target_sources(QtTube PRIVATE
        src/ui/widgets/webengineplayer/fullscreennotification.cpp src/ui/widgets/webengineplayer/fullscreennotification.h
        src/ui/widgets/webengineplayer/fullscreenwindow.cpp src/ui/widgets/webengineplayer/fullscreenwindow.h
        src/ui/widgets/webengineplayer/playerinterceptor.cpp src/ui/widgets/webengineplayer/playerinterceptor.h
        src/ui/widgets/webengineplayer/webchannelinterface.cpp src/ui/widgets/webengineplayer/webchannelinterface.h
        src/ui/widgets/webengineplayer/webengineplayer.cpp src/ui/widgets/webengineplayer/webengineplayer.h
    )
    target_link_libraries(QtTube PRIVATE Qt::WebEngineWidgets)
endif()

## Platform-specific
if(WIN32)
    target_link_libraries(QtTube PRIVATE user32)
elseif(UNIX AND NOT APPLE)
    find_package(X11)
    if(X11_FOUND AND X11_Xscreensaver_LIB)
        target_include_directories(QtTube PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(QtTube PRIVATE ${X11_X11_LIB} ${X11_Xscreensaver_LIB})
        target_compile_definitions(QtTube PUBLIC QTTUBE_HAS_XSS)

        find_library(X11EXTRAS Qt::X11Extras)
        if (X11EXTRAS)
            target_compile_definitions(QtTube PUBLIC QTTUBE_HAS_X11EXTRAS)
            target_link_libraries(QtTube PRIVATE Qt::X11Extras)
        endif()
    else()
        message(STATUS "XScreenSaver not found. Idle sleeping will not be functional.")
    endif()
endif()

# Final stuff
if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

install(TARGETS QtTube
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
