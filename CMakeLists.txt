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
set(QTTUBE_VERSION_MAJOR 0)
set(QTTUBE_VERSION_MINOR 1)
set(QTTUBE_VERSION_NAME "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}")

# Compilation options
option(QTTUBE_ENABLE_ASAN "Enable AddressSanitizer to detect memory errors in debug builds." ON)
option(QTTUBE_EXTERNAL_OPENSSL "Grab OpenSSL externally when building if it's not installed on Windows." ON)
option(QTTUBE_USEMPV "Use MPV for the video player." OFF)

# Configure files
if(WIN32)
    configure_file(program_info/win_install.nsi.in program_info/win_install.nsi @ONLY)
endif()

# Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network Widgets)

if(QT_VERSION_MAJOR EQUAL 5)
    if(UNIX AND NOT APPLE)
        find_package(Qt5 OPTIONAL_COMPONENTS OpenGL WebEngineWidgets X11Extras)
    else()
        find_package(Qt5 OPTIONAL_COMPONENTS OpenGL WebEngineWidgets)
    endif()
elseif(QT_VERSION_MAJOR EQUAL 6)
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
    src/credentialsstore.cpp
    src/osutilities.cpp
    src/qttubeapplication.cpp
    src/settingsstore.cpp
    src/ytemoji.cpp
    src/ui/browsehelper.cpp
    src/ui/channelbrowser.cpp
    src/ui/uiutilities.cpp
    src/ui/forms/emojimenu.cpp
    src/ui/forms/livechatwindow.cpp
    src/ui/forms/mainwindow.cpp
    src/ui/forms/settingsform.cpp
    src/ui/views/channelview.cpp
    src/ui/views/viewcontroller.cpp
    src/ui/views/watchview.cpp
    src/ui/views/watchview_ui.cpp
    src/ui/views/watchviewplayer.cpp
    src/ui/widgets/findbar.cpp
    src/ui/widgets/flowlayout.cpp
    src/ui/widgets/accountmenu/accountcontrollerwidget.cpp
    src/ui/widgets/accountmenu/accountentrywidget.cpp
    src/ui/widgets/accountmenu/accountmenuwidget.cpp
    src/ui/widgets/accountmenu/accountswitcherwidget.cpp
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
    src/credentialsstore.h
    src/osutilities.h
    src/qttubeapplication.h
    src/settingsstore.h
    src/ytemoji.h
    src/ui/browsehelper.h
    src/ui/browsehelper.tpp
    src/ui/channelbrowser.h
    src/ui/uiutilities.h
    src/ui/uiutilities.tpp
    src/ui/forms/emojimenu.h
    src/ui/forms/livechatwindow.h
    src/ui/forms/mainwindow.h
    src/ui/forms/settingsform.h
    src/ui/views/channelview.h
    src/ui/views/viewcontroller.h
    src/ui/views/watchview.h
    src/ui/views/watchview_ui.h
    src/ui/views/watchviewplayer.h
    src/ui/widgets/findbar.h
    src/ui/widgets/flowlayout.h
    src/ui/widgets/accountmenu/accountcontrollerwidget.h
    src/ui/widgets/accountmenu/accountentrywidget.h
    src/ui/widgets/accountmenu/accountmenuwidget.h
    src/ui/widgets/accountmenu/accountswitcherwidget.h
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
    src/ui/forms/emojimenu.ui
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
target_link_libraries(QtTube PRIVATE http innertube-qt Qt::Core Qt::Gui Qt::Network Qt::Widgets)

# Address sanitizer
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND QTTUBE_ENABLE_ASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fsanitize=undefined -fsanitize=address")
    target_link_options(QtTube BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address)
endif()

## Optional
if(QTTUBE_USEMPV)
    target_sources(QtTube PRIVATE
        lib/media/media.h
        lib/media/mpv/mediampv.cpp lib/media/mpv/mediampv.h
        lib/media/mpv/mpvwidget.cpp lib/media/mpv/mpvwidget.h
        lib/media/mpv/qthelper.hpp
    )

    if(QT_VERSION_MAJOR EQUAL 6)
        target_link_libraries(QtTube PRIVATE Qt::OpenGLWidgets)
    else()
        target_link_libraries(QtTube PRIVATE Qt::OpenGL)
    endif()

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

    find_package(OpenSSL QUIET)
    if(OpenSSL_FOUND)
        target_include_directories(QtTube PRIVATE ${OPENSSL_INCLUDE_DIR})
        target_link_libraries(QtTube PRIVATE ${OPENSSL_LIBRARIES})
    elseif(QTTUBE_EXTERNAL_OPENSSL)
        # https://stackoverflow.com/a/72187679
        set(OPENSSL_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-src) # default path by CMake
        set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl)
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
        set(OPENSSL_CONFIGURE_COMMAND ${OPENSSL_SOURCE_DIR}/Configure)

        file(GLOB SSLGLOB CONFIGURE_DEPENDS ${OPENSSL_INSTALL_DIR}/bin/*.dll)
        list(LENGTH SSLGLOB OPENSSL_INSTALL_LEN)

        if(OPENSSL_INSTALL_LEN EQUAL 0)
            message(STATUS "OpenSSL could not be found on the system, will be grabbed externally.")
            include(ExternalProject)

            ExternalProject_Add(
              OpenSSL
              SOURCE_DIR ${OPENSSL_SOURCE_DIR}
              GIT_REPOSITORY https://github.com/openssl/openssl.git
              GIT_TAG OpenSSL_1_1_1n
              USES_TERMINAL_DOWNLOAD TRUE
              CONFIGURE_COMMAND
                perl ${OPENSSL_CONFIGURE_COMMAND} VC-WIN64A
                --prefix=${OPENSSL_INSTALL_DIR}
                --openssldir=${OPENSSL_INSTALL_DIR}
              BUILD_COMMAND nmake
              TEST_COMMAND ""
              INSTALL_COMMAND nmake install
              INSTALL_DIR ${OPENSSL_INSTALL_DIR}
              BUILD_BYPRODUCTS ${OPENSSL_INSTALL_DIR}/lib/libssl.lib ${OPENSSL_INSTALL_DIR}/lib/libcrypto.lib
            )
        endif()

        # We cannot use find_library because ExternalProject_Add() is performed at build time.
        # And to please the property INTERFACE_INCLUDE_DIRECTORIES,
        # we make the include directory in advance.
        file(MAKE_DIRECTORY ${OPENSSL_INCLUDE_DIR})

        add_library(OpenSSL::SSL STATIC IMPORTED GLOBAL)
        set_property(TARGET OpenSSL::SSL PROPERTY IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/libssl.lib)
        set_property(TARGET OpenSSL::SSL PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})
        add_dependencies(OpenSSL::SSL OpenSSL)

        add_library(OpenSSL::Crypto STATIC IMPORTED GLOBAL)
        set_property(TARGET OpenSSL::Crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/libcrypto.lib)
        set_property(TARGET OpenSSL::Crypto PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})
        add_dependencies(OpenSSL::Crypto OpenSSL)

        target_link_libraries(QtTube PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
elseif(UNIX AND NOT APPLE)
    find_package(X11)
    if(X11_FOUND AND X11_Xscreensaver_LIB)
        target_include_directories(QtTube PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(QtTube PRIVATE ${X11_X11_LIB} ${X11_Xscreensaver_LIB})
        target_compile_definitions(QtTube PUBLIC QTTUBE_HAS_XSS)

        find_library(X11EXTRAS Qt::X11Extras)
        if(X11EXTRAS)
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
