project(plugin_interface)

find_package(Qt${QTTUBE_QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(QtTube-PluginInterface STATIC)

target_compile_definitions(QtTube-PluginInterface PUBLIC
    QTTUBE_VERSION_NAME="${QTTUBE_VERSION_NAME}")

target_sources(QtTube-PluginInterface
    PRIVATE
        qttube-plugin/components/pluginsettingswindow.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            qttube-plugin/plugininterface.h
            qttube-plugin/components/configstore.h
            qttube-plugin/components/pluginauth.h
            qttube-plugin/components/pluginsettings.h
            qttube-plugin/components/pluginsettingswindow.h)

target_link_libraries(QtTube-PluginInterface PUBLIC Qt::Widgets)
add_library(QtTube::PluginInterface ALIAS QtTube-PluginInterface)
