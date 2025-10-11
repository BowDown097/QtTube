# Qt packages
find_package(Qt6 REQUIRED COMPONENTS Network WebEngineWidgets Widgets)

# Address sanitizer
option(QTTUBE_ENABLE_ASAN "Enable AddressSanitizer to detect memory errors in debug builds." OFF)
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND QTTUBE_ENABLE_ASAN)
    message(STATUS "Enabling ASAN")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/mainwindow.cpp
    src/qttubeapplication.cpp
    src/plugins/pluginbrowser.cpp
    src/plugins/pluginmanager.cpp
    src/stores/emojistore.cpp
    src/stores/settingsstore.cpp
    src/ui/browsehelper.cpp
    src/ui/forms/livechatwindow.cpp
    src/ui/forms/plugins/addplugindialog.cpp
    src/ui/forms/plugins/basepluginentry.cpp
    src/ui/forms/plugins/pluginbrowserview.cpp
    src/ui/forms/plugins/pluginbuilddownloader.cpp
    src/ui/forms/settings/settingsform.cpp
    src/ui/forms/settings/termfilterview.cpp
    src/ui/views/channelview.cpp
    src/ui/views/viewcontroller.cpp
    src/ui/views/watchview.cpp
    src/ui/views/watchview_ui.cpp
    src/ui/views/watchviewplayer.cpp
    src/ui/widgets/closebutton.cpp
    src/ui/widgets/continuablelistwidget.cpp
    src/ui/widgets/dynamiclistwidgetitem.cpp
    src/ui/widgets/findbar.cpp
    src/ui/widgets/watchnextfeed.cpp
    src/ui/widgets/accountmenu/accountcontrollerwidget.cpp
    src/ui/widgets/accountmenu/accountentrywidget.cpp
    src/ui/widgets/accountmenu/accountmenuwidget.cpp
    src/ui/widgets/accountmenu/accountswitcherwidget.cpp
    src/ui/widgets/emoji/cachednetworkworker.cpp
    src/ui/widgets/emoji/emojigraphicsitem.cpp
    src/ui/widgets/emoji/emojimenu.cpp
    src/ui/widgets/labels/badgelabel.cpp
    src/ui/widgets/labels/channellabel.cpp
    src/ui/widgets/labels/iconlabel.cpp
    src/ui/widgets/labels/tubelabel.cpp
    src/ui/widgets/livechat/giftredemptionmessage.cpp
    src/ui/widgets/livechat/paidmessage.cpp
    src/ui/widgets/livechat/specialmessage.cpp
    src/ui/widgets/livechat/textmessage.cpp
    src/ui/widgets/modals/focusedmodal.cpp
    src/ui/widgets/modals/sharemodal.cpp
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
    src/utils/osutils.cpp
    src/utils/uiutils.cpp
    res/resources.qrc
)

set(HEADERS
    src/mainwindow.h
    src/qttubeapplication.h
    src/plugins/pluginbrowser.h
    src/plugins/pluginbrowsercache.h
    src/plugins/pluginmanager.h
    src/stores/emojistore.h
    src/stores/settingsstore.h
    src/ui/browsehelper.h
    src/ui/forms/livechatwindow.h
    src/ui/forms/plugins/addplugindialog.h
    src/ui/forms/plugins/basepluginentry.h
    src/ui/forms/plugins/pluginbrowserview.h
    src/ui/forms/plugins/pluginbuilddownloader.h
    src/ui/forms/settings/settingsform.h
    src/ui/forms/settings/termfilterview.h
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
    src/ui/widgets/watchnextfeed.h
    src/ui/widgets/accountmenu/accountcontrollerwidget.h
    src/ui/widgets/accountmenu/accountentrywidget.h
    src/ui/widgets/accountmenu/accountmenuwidget.h
    src/ui/widgets/accountmenu/accountswitcherwidget.h
    src/ui/widgets/emoji/cachednetworkworker.h
    src/ui/widgets/emoji/emojigraphicsitem.h
    src/ui/widgets/emoji/emojimenu.h
    src/ui/widgets/labels/badgelabel.h
    src/ui/widgets/labels/channellabel.h
    src/ui/widgets/labels/iconlabel.h
    src/ui/widgets/labels/tubelabel.h
    src/ui/widgets/livechat/giftredemptionmessage.h
    src/ui/widgets/livechat/paidmessage.h
    src/ui/widgets/livechat/specialmessage.h
    src/ui/widgets/livechat/textmessage.h
    src/ui/widgets/modals/focusedmodal.h
    src/ui/widgets/modals/sharemodal.h
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
    src/utils/osutils.h
    src/utils/uiutils.h
)

set(FORMS
    src/mainwindow.ui
    src/ui/forms/livechatwindow.ui
    src/ui/forms/plugins/addplugindialog.ui
    src/ui/forms/plugins/pluginbrowserview.ui
    src/ui/forms/settings/settingsform.ui
    src/ui/forms/settings/termfilterview.ui
    src/ui/widgets/emoji/emojimenu.ui
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
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::GuiPrivate QtTube::PluginInterface)

## HTML parser
include(FetchContent)
FetchContent_Declare(
    HtmlParser
    GIT_REPOSITORY https://github.com/JustCabbage/HtmlParser.git
    GIT_TAG master)

option(HTMLPARSER_BUILD_TESTS "Build tests" OFF)
option(HTMLPARSER_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(HTMLPARSER_BUILD_EXAMPLES "Build examples" OFF)
FetchContent_MakeAvailable(HtmlParser)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE HtmlParser)

## Platform-specific
if(UNIX AND NOT APPLE)
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

## Application
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME "${QTTUBE_BINARY_NAME}")
install(TARGETS ${CMAKE_PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
