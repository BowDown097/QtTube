project(youtube_plugin)
add_library(youtube_plugin SHARED)

target_sources(youtube_plugin
    PRIVATE
        main.cpp
        youtubesettings.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            youtubesettings.h)

target_link_libraries(${PROJECT_NAME} PUBLIC QtTube::PluginInterface)

set_target_properties(youtube_plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
