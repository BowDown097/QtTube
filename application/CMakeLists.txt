qt_add_executable(QtTube WIN32 MACOSX_BUNDLE)

# Address sanitizer
option(QTTUBE_ENABLE_ASAN "Enable AddressSanitizer to detect memory errors in debug builds" OFF)
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND QTTUBE_ENABLE_ASAN)
    message(STATUS "Enabling ASAN")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Sources
target_sources(QtTube
    PRIVATE
        # source files
        src/main.cpp
        src/mainwindow.cpp
        src/qttubeapplication.cpp
        src/plugins/pluginbrowser.cpp
        src/plugins/pluginbuilddownloader.cpp
        src/plugins/pluginentry.cpp
        src/plugins/pluginmanager.cpp
        src/plugins/scripted/scriptpluginauthstore.cpp
        src/plugins/scripted/scriptpluginentry.cpp
        src/plugins/scripted/scriptplugininterface.cpp
        src/stores/emojistore.cpp
        src/stores/settingsstore.cpp
        src/ui/browsehelper.cpp
        src/ui/forms/livechatwindow.cpp
        src/ui/forms/plugins/addplugindialog.cpp
        src/ui/forms/plugins/basepluginentry.cpp
        src/ui/forms/plugins/pluginbrowserview.cpp
        src/ui/forms/plugins/plugindownloaddialog.cpp
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
        src/ui/widgets/wrappers/popupwidget.cpp
        src/utils/osutils.cpp
        src/utils/uiutils.cpp
        src/utils/quickjs/plugin_js_traits.cpp
        src/utils/quickjs/qt_js_traits.cpp
        src/utils/quickjs/polyfills/fetch.cpp
        src/utils/quickjs/polyfills/intl.cpp
        src/utils/quickjs/polyfills/navigator.cpp
        # forms, etc.
        res/resources.qrc
        src/mainwindow.ui
        src/ui/forms/livechatwindow.ui
        src/ui/forms/plugins/addplugindialog.ui
        src/ui/forms/plugins/pluginbrowserview.ui
        src/ui/forms/settings/settingsform.ui
        src/ui/forms/settings/termfilterview.ui
        src/ui/widgets/emoji/emojimenu.ui
    PRIVATE FILE_SET HEADERS FILES
        src/mainwindow.hpp
        src/qttubeapplication.hpp
        src/plugins/pluginbrowser.hpp
        src/plugins/pluginbrowsercache.hpp
        src/plugins/pluginbuilddownloader.hpp
        src/plugins/pluginentry.hpp
        src/plugins/pluginmanager.hpp
        src/plugins/releasedata.hpp
        src/plugins/scripted/scriptpluginauthstore.hpp
        src/plugins/scripted/scriptpluginentry.hpp
        src/plugins/scripted/scriptplugininterface.hpp
        src/stores/emojistore.hpp
        src/stores/settingsstore.hpp
        src/ui/browsehelper.hpp
        src/ui/forms/livechatwindow.hpp
        src/ui/forms/plugins/addplugindialog.hpp
        src/ui/forms/plugins/basepluginentry.hpp
        src/ui/forms/plugins/pluginbrowserview.hpp
        src/ui/forms/plugins/plugindownloaddialog.hpp
        src/ui/forms/settings/settingsform.hpp
        src/ui/forms/settings/termfilterview.hpp
        src/ui/views/channelview.hpp
        src/ui/views/preloaddata.hpp
        src/ui/views/viewcontroller.hpp
        src/ui/views/watchview.hpp
        src/ui/views/watchview_ui.hpp
        src/ui/views/watchviewplayer.hpp
        src/ui/widgets/closebutton.hpp
        src/ui/widgets/continuablelistwidget.hpp
        src/ui/widgets/dynamiclistwidgetitem.hpp
        src/ui/widgets/findbar.hpp
        src/ui/widgets/watchnextfeed.hpp
        src/ui/widgets/accountmenu/accountcontrollerwidget.hpp
        src/ui/widgets/accountmenu/accountentrywidget.hpp
        src/ui/widgets/accountmenu/accountmenuwidget.hpp
        src/ui/widgets/accountmenu/accountswitcherwidget.hpp
        src/ui/widgets/emoji/cachednetworkworker.hpp
        src/ui/widgets/emoji/emojigraphicsitem.hpp
        src/ui/widgets/emoji/emojimenu.hpp
        src/ui/widgets/labels/badgelabel.hpp
        src/ui/widgets/labels/channellabel.hpp
        src/ui/widgets/labels/iconlabel.hpp
        src/ui/widgets/labels/tubelabel.hpp
        src/ui/widgets/livechat/giftredemptionmessage.hpp
        src/ui/widgets/livechat/paidmessage.hpp
        src/ui/widgets/livechat/specialmessage.hpp
        src/ui/widgets/livechat/textmessage.hpp
        src/ui/widgets/modals/focusedmodal.hpp
        src/ui/widgets/modals/sharemodal.hpp
        src/ui/widgets/renderers/browsechannelrenderer.hpp
        src/ui/widgets/renderers/browsenotificationrenderer.hpp
        src/ui/widgets/renderers/video/browsevideorenderer.hpp
        src/ui/widgets/renderers/video/gridvideorenderer.hpp
        src/ui/widgets/renderers/video/videorenderer.hpp
        src/ui/widgets/renderers/video/videothumbnailwidget.hpp
        src/ui/widgets/subscribe/notificationbell.hpp
        src/ui/widgets/subscribe/subscribelabel.hpp
        src/ui/widgets/subscribe/subscribewidget.hpp
        src/ui/widgets/topbar/extmenu.hpp
        src/ui/widgets/topbar/exttoolbutton.hpp
        src/ui/widgets/topbar/searchbox.hpp
        src/ui/widgets/topbar/topbar.hpp
        src/ui/widgets/topbar/topbarbell.hpp
        src/ui/widgets/wrappers/clickablewidget.hpp
        src/ui/widgets/wrappers/popupwidget.hpp
        src/utils/hashfunctions.hpp
        src/utils/osutils.hpp
        src/utils/uiutils.hpp
        src/utils/quickjs/plugin_js_traits.hpp
        src/utils/quickjs/qjsutils.hpp
        src/utils/quickjs/qt_js_traits.hpp
        src/utils/quickjs/polyfills/fetch.hpp
        src/utils/quickjs/polyfills/intl.hpp
        src/utils/quickjs/polyfills/navigator.hpp
)

# Application icon
if(WIN32 OR APPLE)
    include(cmake/AddIconToBinary.cmake)
    AddIconToBinary(SOURCE_FILES ICONS res/qttube.ico res/qttube.icns)
endif()

# Preprocessor definitions
execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE QTTUBE_BRANCH)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE QTTUBE_COMMIT_ID)
target_compile_definitions(QtTube PRIVATE
    QTTUBE_APP_DESC="${QTTUBE_APP_DESC}"
    QTTUBE_APP_NAME="${QTTUBE_APP_NAME}"
    QTTUBE_BRANCH="${QTTUBE_BRANCH}"
    QTTUBE_COMMIT_ID="${QTTUBE_COMMIT_ID}"
    QTTUBE_REPO_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
)

# Include directories, libraries
## Qt
find_package(Qt6 REQUIRED COMPONENTS Network WebEngineWidgets Widgets)
target_link_libraries(QtTube PRIVATE Qt::Network Qt::WebEngineWidgets Qt::Widgets)
## Built-in
target_include_directories(QtTube PRIVATE src)
target_link_libraries(QtTube PRIVATE QuaZip quickjs++ QtTube::PluginInterface)
## Platform-specific (Unix)
if(UNIX AND NOT APPLE)
    # Xscreensaver
    find_package(X11 QUIET)
    if(X11_FOUND AND X11_Xscreensaver_LIB)
        target_include_directories(QtTube PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(QtTube PRIVATE ${X11_X11_LIB} ${X11_Xscreensaver_LIB})
        target_compile_definitions(QtTube PRIVATE QTTUBE_HAS_XSS)
        if(TARGET Qt::X11Extras)
            target_compile_definitions(QtTube PRIVATE QTTUBE_HAS_X11EXTRAS)
            target_link_libraries(QtTube PRIVATE Qt::X11Extras)
        endif()
    else()
        message(STATUS "XScreenSaver not found. Idle sleep will not be inhibited on X11 in this build.")
    endif()
    # Wayland
    find_package(Wayland QUIET)
    if(Wayland_FOUND)
        target_sources(QtTube
            PRIVATE
                src/wayland/ext-idle-inhibit-unstable-v1.c
                src/wayland/waylandinterface.cpp
            PRIVATE FILE_SET HEADERS FILES
                src/wayland/ext-idle-inhibit-unstable-v1.h
                src/wayland/waylandinterface.hpp
        )
        target_compile_definitions(QtTube PRIVATE QTTUBE_HAS_WAYLAND)
        target_include_directories(QtTube PRIVATE ${Wayland_INCLUDE_DIRS})
        target_link_libraries(QtTube PRIVATE ${Wayland_LIBRARIES})
        # needed for QPlatformNativeInterface on Qt < 6.5
        if(Qt6Core_VERSION VERSION_LESS 6.5.0)
            target_link_libraries(QtTube PRIVATE Qt::GuiPrivate)
        endif()
    endif()
endif()
## ICU
find_package(ICU REQUIRED COMPONENTS uc i18n)
target_link_libraries(QtTube PRIVATE ICU::uc ICU::i18n)

# Installation
set_target_properties(QtTube PROPERTIES OUTPUT_NAME "${QTTUBE_BINARY_NAME}")
install(TARGETS QtTube
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
