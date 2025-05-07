project(plugin_interface)

find_package(Qt${QTTUBE_QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(QtTube-PluginInterface)

target_compile_definitions(QtTube-PluginInterface PUBLIC
    QTTUBE_VERSION_NAME="${QTTUBE_VERSION_NAME}")

target_sources(QtTube-PluginInterface
    PUBLIC
        FILE_SET HEADERS FILES
            qttube-plugin/plugininterface.h)

target_link_libraries(QtTube-PluginInterface PUBLIC Qt::Core)
add_library(QtTube::PluginInterface ALIAS QtTube-PluginInterface)
