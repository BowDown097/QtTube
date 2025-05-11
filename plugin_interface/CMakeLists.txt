project(plugin_interface)

find_package(Qt${QTTUBE_QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(QtTube-PluginInterface INTERFACE)

target_compile_definitions(QtTube-PluginInterface INTERFACE
    QTTUBE_VERSION_NAME="${QTTUBE_VERSION_NAME}")

target_sources(QtTube-PluginInterface
    PUBLIC
        FILE_SET HEADERS FILES
            qttube-plugin/plugininterface.h
            qttube-plugin/components/configstore.h
            qttube-plugin/components/pluginauth.h
            qttube-plugin/components/pluginsettings.h)

target_link_libraries(QtTube-PluginInterface INTERFACE Qt::Core)
add_library(QtTube::PluginInterface ALIAS QtTube-PluginInterface)
