cmake_minimum_required(VERSION 3.23)
project(QtTube
    VERSION 0.1
    DESCRIPTION "A Qt frontend for YouTube."
    HOMEPAGE_URL "https://github.com/BowDown097/QtTube"
    LANGUAGES CXX C)

# Have to do this for ui_ files
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC, AUTORCC, and AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Names
## App
set(QTTUBE_APP_DESC "${CMAKE_PROJECT_DESCRIPTION}")
set(QTTUBE_APP_NAME "${CMAKE_PROJECT_NAME}")
## Version name
set(QTTUBE_VERSION_MAJOR "${CMAKE_PROJECT_VERSION_MAJOR}")
set(QTTUBE_VERSION_MINOR "${CMAKE_PROJECT_VERSION_MINOR}")
set(QTTUBE_VERSION_NAME "${CMAKE_PROJECT_VERSION}")
if(CMAKE_PROJECT_VERSION_TWEAK)
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.\
                              ${CMAKE_PROJECT_VERSION_PATCH}.${CMAKE_PROJECT_VERSION_TWEAK}")
elseif(CMAKE_PROJECT_VERSION_PATCH)
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.\
                              ${CMAKE_PROJECT_VERSION_PATCH}.0")
else()
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.0.0")
endif()

# Compilation options
option(QTTUBE_ENABLE_ASAN "Enable AddressSanitizer to detect memory errors in debug builds." OFF)
option(QTTUBE_EXTERNAL_OPENSSL "Grab OpenSSL externally when building if it's not installed on Windows." ON)

# Address sanitizer
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND QTTUBE_ENABLE_ASAN)
    message(STATUS "Enabling ASAN")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Configure files
if(WIN32)
    configure_file(program_info/win_install.nsi.in program_info/win_install.nsi @ONLY)
endif()

# Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
set(QTTUBE_QT_VERSION_MAJOR ${QT_VERSION_MAJOR} CACHE STRING "Qt version to use, defaults to ${QT_VERSION_MAJOR}")
find_package(Qt${QTTUBE_QT_VERSION_MAJOR} REQUIRED COMPONENTS Network Sql WebEngineWidgets Widgets)

if(QTTUBE_QT_VERSION_MAJOR EQUAL 5 AND (UNIX AND NOT APPLE))
    find_package(Qt5 OPTIONAL_COMPONENTS X11Extras)
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
    src/eastereggs.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/qttubeapplication.cpp
    src/ytemoji.cpp
    src/stores/credentialsstore.cpp
    src/stores/genericstore.cpp
    src/stores/settingsstore.cpp
    src/ui/browsehelper.cpp
    src/ui/channelbrowser.cpp
    src/ui/forms/emojimenu.cpp
    src/ui/forms/livechat/giftredemptionmessage.cpp
    src/ui/forms/livechat/livechatwindow.cpp
    src/ui/forms/livechat/paidmessage.cpp
    src/ui/forms/livechat/specialmessage.cpp
    src/ui/forms/livechat/textmessage.cpp
    src/ui/forms/settings/channelfiltertable.cpp
    src/ui/forms/settings/settingsform.cpp
    src/ui/forms/settings/termfilterview.cpp
    src/ui/forms/settings/data-wizards/basicinfopage.cpp
    src/ui/forms/settings/data-wizards/datawizard.cpp
    src/ui/forms/settings/data-wizards/entityselecttablemodel.cpp
    src/ui/forms/settings/data-wizards/intropage.cpp
    src/ui/forms/settings/data-wizards/import/grayjayimportwizard.cpp
    src/ui/forms/settings/data-wizards/import/newpipeimportwizard.cpp
    src/ui/forms/settings/data-wizards/import/pipedimportwizard.cpp
    src/ui/forms/settings/data-wizards/import/takeoutimportwizard.cpp
    src/ui/forms/settings/data-wizards/import/shared/chooseentitiespage.cpp
    src/ui/forms/settings/data-wizards/import/shared/choosesubspage.cpp
    src/ui/forms/settings/data-wizards/import/shared/choosewatchhistorypage.cpp
    src/ui/forms/settings/data-wizards/import/shared/importfileselectpage.cpp
    src/ui/forms/settings/data-wizards/richtableview/richitemdelegate.cpp
    src/ui/forms/settings/data-wizards/richtableview/richtableview.cpp
    src/ui/views/channelview.cpp
    src/ui/views/viewcontroller.cpp
    src/ui/views/watchview.cpp
    src/ui/views/watchview_ui.cpp
    src/ui/views/watchviewplayer.cpp
    src/ui/widgets/closebutton.cpp
    src/ui/widgets/continuablelistwidget.cpp
    src/ui/widgets/dynamiclistwidgetitem.cpp
    src/ui/widgets/findbar.cpp
    src/ui/widgets/flowlayout.cpp
    src/ui/widgets/watchnextfeed.cpp
    src/ui/widgets/accountmenu/accountcontrollerwidget.cpp
    src/ui/widgets/accountmenu/accountentrywidget.cpp
    src/ui/widgets/accountmenu/accountmenuwidget.cpp
    src/ui/widgets/accountmenu/accountswitcherwidget.cpp
    src/ui/widgets/download/downloadentity.cpp
    src/ui/widgets/download/downloadmanager.cpp
    src/ui/widgets/labels/channelbadgelabel.cpp
    src/ui/widgets/labels/channellabel.cpp
    src/ui/widgets/labels/emojilabel.cpp
    src/ui/widgets/labels/iconlabel.cpp
    src/ui/widgets/labels/tubelabel.cpp
    src/ui/widgets/renderers/backstage/backstagepollchoicerenderer.cpp
    src/ui/widgets/renderers/backstage/backstagepollrenderer.cpp
    src/ui/widgets/renderers/backstage/backstagepostrenderer.cpp
    src/ui/widgets/renderers/backstage/backstagequizchoicerenderer.cpp
    src/ui/widgets/renderers/backstage/backstagequizexplanation.cpp
    src/ui/widgets/renderers/backstage/backstagequizrenderer.cpp
    src/ui/widgets/renderers/backstage/basepostrenderer.cpp
    src/ui/widgets/renderers/backstage/postrenderer.cpp
    src/ui/widgets/renderers/browsechannelrenderer.cpp
    src/ui/widgets/renderers/browsenotificationrenderer.cpp
    src/ui/widgets/renderers/video/browsevideorenderer.cpp
    src/ui/widgets/renderers/video/gridvideorenderer.cpp
    src/ui/widgets/renderers/video/videorenderer.cpp
    src/ui/widgets/renderers/video/videothumbnailwidget.cpp
    src/ui/widgets/subscribe/notificationbell.cpp
    src/ui/widgets/subscribe/subscribelabel.cpp
    src/ui/widgets/subscribe/subscribewidget.cpp
    src/ui/widgets/topbar/extmenu.cpp
    src/ui/widgets/topbar/exttoolbutton.cpp
    src/ui/widgets/topbar/searchbox.cpp
    src/ui/widgets/topbar/topbar.cpp
    src/ui/widgets/topbar/topbarbell.cpp
    src/ui/widgets/webengineplayer/fullscreennotification.cpp
    src/ui/widgets/webengineplayer/fullscreenwindow.cpp
    src/ui/widgets/webengineplayer/playerinterceptor.cpp
    src/ui/widgets/webengineplayer/webchannelinterface.cpp
    src/ui/widgets/webengineplayer/webengineplayer.cpp
    src/utils/httputils.cpp
    src/utils/innertubestringformatter.cpp
    src/utils/osutils.cpp
    src/utils/stringutils.cpp
    src/utils/tubeutils.cpp
    src/utils/uiutils.cpp
    res/resources.qrc
)

set(HEADERS
    src/eastereggs.h
    src/mainwindow.h
    src/qttubeapplication.h
    src/ytemoji.h
    src/stores/credentialsstore.h
    src/stores/genericstore.h
    src/stores/settingsstore.h
    src/ui/browsehelper.h
    src/ui/channelbrowser.h
    src/ui/forms/emojimenu.h
    src/ui/forms/livechat/giftredemptionmessage.h
    src/ui/forms/livechat/livechatwindow.h
    src/ui/forms/livechat/paidmessage.h
    src/ui/forms/livechat/specialmessage.h
    src/ui/forms/livechat/textmessage.h
    src/ui/forms/settings/channelfiltertable.h
    src/ui/forms/settings/settingsform.h
    src/ui/forms/settings/termfilterview.h
    src/ui/forms/settings/data-wizards/basicinfopage.h
    src/ui/forms/settings/data-wizards/conclusionpage.h
    src/ui/forms/settings/data-wizards/datawizard.h
    src/ui/forms/settings/data-wizards/entityselecttablemodel.h
    src/ui/forms/settings/data-wizards/intropage.h
    src/ui/forms/settings/data-wizards/import/grayjayimportwizard.h
    src/ui/forms/settings/data-wizards/import/newpipeimportwizard.h
    src/ui/forms/settings/data-wizards/import/pipedimportwizard.h
    src/ui/forms/settings/data-wizards/import/takeoutimportwizard.h
    src/ui/forms/settings/data-wizards/import/shared/chooseentitiespage.h
    src/ui/forms/settings/data-wizards/import/shared/choosesubspage.h
    src/ui/forms/settings/data-wizards/import/shared/choosewatchhistorypage.h
    src/ui/forms/settings/data-wizards/import/shared/importfileselectpage.h
    src/ui/forms/settings/data-wizards/richtableview/richitemdelegate.h
    src/ui/forms/settings/data-wizards/richtableview/richtableview.h
    src/ui/views/channelview.h
    src/ui/views/preloaddata.h
    src/ui/views/viewcontroller.h
    src/ui/views/watchview.h
    src/ui/views/watchview_ui.h
    src/ui/views/watchviewplayer.h
    src/ui/widgets/clickablewidget.h
    src/ui/widgets/closebutton.h
    src/ui/widgets/continuablelistwidget.h
    src/ui/widgets/dynamiclistwidgetitem.h
    src/ui/widgets/findbar.h
    src/ui/widgets/flowlayout.h
    src/ui/widgets/watchnextfeed.h
    src/ui/widgets/accountmenu/accountcontrollerwidget.h
    src/ui/widgets/accountmenu/accountentrywidget.h
    src/ui/widgets/accountmenu/accountmenuwidget.h
    src/ui/widgets/accountmenu/accountswitcherwidget.h
    src/ui/widgets/download/downloadentity.h
    src/ui/widgets/download/downloadmanager.h
    src/ui/widgets/labels/channelbadgelabel.h
    src/ui/widgets/labels/channellabel.h
    src/ui/widgets/labels/emojilabel.h
    src/ui/widgets/labels/iconlabel.h
    src/ui/widgets/labels/tubelabel.h
    src/ui/widgets/renderers/backstage/backstagepollchoicerenderer.h
    src/ui/widgets/renderers/backstage/backstagepollrenderer.h
    src/ui/widgets/renderers/backstage/backstagepostrenderer.h
    src/ui/widgets/renderers/backstage/backstagequizchoicerenderer.h
    src/ui/widgets/renderers/backstage/backstagequizexplanation.h
    src/ui/widgets/renderers/backstage/backstagequizrenderer.h
    src/ui/widgets/renderers/backstage/basepostrenderer.h
    src/ui/widgets/renderers/backstage/postrenderer.h
    src/ui/widgets/renderers/browsechannelrenderer.h
    src/ui/widgets/renderers/browsenotificationrenderer.h
    src/ui/widgets/renderers/video/browsevideorenderer.h
    src/ui/widgets/renderers/video/gridvideorenderer.h
    src/ui/widgets/renderers/video/videorenderer.h
    src/ui/widgets/renderers/video/videothumbnailwidget.h
    src/ui/widgets/subscribe/notificationbell.h
    src/ui/widgets/subscribe/subscribelabel.h
    src/ui/widgets/subscribe/subscribewidget.h
    src/ui/widgets/topbar/extmenu.h
    src/ui/widgets/topbar/exttoolbutton.h
    src/ui/widgets/topbar/searchbox.h
    src/ui/widgets/topbar/topbar.h
    src/ui/widgets/topbar/topbarbell.h
    src/ui/widgets/webengineplayer/fullscreennotification.h
    src/ui/widgets/webengineplayer/fullscreenwindow.h
    src/ui/widgets/webengineplayer/playerinterceptor.h
    src/ui/widgets/webengineplayer/webchannelinterface.h
    src/ui/widgets/webengineplayer/webengineplayer.h
    src/utils/httputils.h
    src/utils/innertubestringformatter.h
    src/utils/osutils.h
    src/utils/stringutils.h
    src/utils/tubeutils.h
    src/utils/uiutils.h
)

set(FORMS
    src/mainwindow.ui
    src/ui/forms/emojimenu.ui
    src/ui/forms/livechat/livechatwindow.ui
    src/ui/forms/settings/channelfiltertable.ui
    src/ui/forms/settings/settingsform.ui
    src/ui/forms/settings/termfilterview.ui
)

# Wayland idle-inhibit stuff
if(UNIX AND NOT APPLE)
    find_package(Wayland QUIET)
    if(Wayland_FOUND)
        list(APPEND SOURCE_FILES
            src/wayland/ext-idle-inhibit-unstable-v1.c
            src/wayland/waylandinterface.cpp)
        list(APPEND HEADERS
            src/wayland/ext-idle-inhibit-unstable-v1.h
            src/wayland/waylandinterface.h)
    endif()
endif()

# Application icon
if(WIN32 OR APPLE)
    include(cmake/AddIconToBinary.cmake)
    AddIconToBinary(SOURCE_FILES ICONS res/qttube.ico res/qttube.icns)
endif()

# Add executable
set(INNERTUBE_QT_VERSION_MAJOR ${QTTUBE_QT_VERSION_MAJOR} CACHE STRING "Qt version to use, defaults to ${QTTUBE_QT_VERSION_MAJOR}" FORCE)
add_subdirectory(lib/innertube-qt)
if(WIN32)
    add_executable(${CMAKE_PROJECT_NAME} WIN32 ${SOURCE_FILES} ${HEADERS} ${FORMS})
elseif(APPLE)
    add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES} ${HEADERS} ${FORMS})
else()
    add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES} ${HEADERS} ${FORMS})
endif()

# Wayland idle-inhibit stuff (continued)
if(Wayland_FOUND)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE QTTUBE_HAS_WAYLAND)

    if(QTTUBE_QT_VERSION_MAJOR EQUAL 5)
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${Qt${QTTUBE_QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS})
    else()
        target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::GuiPrivate)
    endif()

    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${Wayland_INCLUDE_DIRS})
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${Wayland_LIBRARIES})
endif()

# Preprocessor definitions
execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE QTTUBE_BRANCH)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE QTTUBE_COMMIT_ID)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    QTTUBE_APP_DESC="${QTTUBE_APP_DESC}"
    QTTUBE_APP_NAME="${QTTUBE_APP_NAME}"
    QTTUBE_BRANCH="${QTTUBE_BRANCH}"
    QTTUBE_COMMIT_ID="${QTTUBE_COMMIT_ID}"
    QTTUBE_REPO_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
    QTTUBE_VERSION_NAME="${QTTUBE_VERSION_NAME}"
)

# Include directories, libraries
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE innertube-qt Qt::Network Qt::Sql Qt::WebEngineWidgets Qt::Widgets)

## Optional
find_package(ICU COMPONENTS i18n uc)
if(ICU_FOUND)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE QTTUBE_HAS_ICU)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${ICU_INCLUDE_DIRS})
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${ICU_LIBRARIES})
endif()

## Platform-specific
if(WIN32)
    find_package(OpenSSL QUIET)
    if(OpenSSL_FOUND)
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
        target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES})
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

        target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
elseif(UNIX AND NOT APPLE)
    find_package(X11 QUIET)
    if(X11_FOUND AND X11_Xscreensaver_LIB)
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${X11_X11_LIB} ${X11_Xscreensaver_LIB})
        target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE QTTUBE_HAS_XSS)

        if(TARGET Qt::X11Extras)
            target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE QTTUBE_HAS_X11EXTRAS)
            target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::X11Extras)
        endif()
    else()
        message(STATUS "XScreenSaver not found. Idle sleeping will not be functional.")
    endif()
endif()

# Installation
include(GNUInstallDirs)

if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

install(TARGETS QtTube
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
