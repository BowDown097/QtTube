project(youtube_plugin)
add_library(youtube_plugin SHARED)

target_sources(youtube_plugin
    PRIVATE
        main.cpp
        youtubeauth.cpp
        youtubesettings.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            youtubeauth.h
            youtubesettings.h)

set(INNERTUBE_QT_VERSION_MAJOR ${QTTUBE_QT_VERSION_MAJOR} CACHE STRING "Qt version to use, defaults to ${QTTUBE_QT_VERSION_MAJOR}" FORCE)
add_subdirectory(innertube-qt)

target_link_libraries(${PROJECT_NAME} PUBLIC innertube-qt QtTube::PluginInterface)

set_target_properties(youtube_plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
