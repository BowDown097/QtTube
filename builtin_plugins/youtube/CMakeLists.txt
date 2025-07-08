project(youtube_plugin)

find_package(Qt${QTTUBE_QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(youtube_plugin SHARED)

find_package(ICU COMPONENTS i18n uc QUIET)
if(ICU_FOUND)
    target_compile_definitions(youtube_plugin PRIVATE QTTUBE_HAS_ICU)
    target_include_directories(youtube_plugin PRIVATE ${ICU_INCLUDE_DIRS})
    target_link_libraries(youtube_plugin PRIVATE ${ICU_LIBRARIES})
endif()

target_link_libraries(youtube_plugin PRIVATE innertube-qt Qt::Sql QtTube::PluginInterface)

target_sources(youtube_plugin
    PRIVATE
        youtubeauth.cpp
        youtubeplayer.cpp
        youtubeplugin.cpp
        res/resources.qrc
        settings/channelfiltertable.cpp
        settings/channelfiltertable.ui
        settings/youtubesettings.cpp
        settings/youtubesettingsform.cpp
        settings/youtubesettingsform.ui
        settings/data-wizards/basicinfopage.cpp
        settings/data-wizards/datawizard.cpp
        settings/data-wizards/entityselecttablemodel.cpp
        settings/data-wizards/intropage.cpp
        settings/data-wizards/import/grayjayimportwizard.cpp
        settings/data-wizards/import/newpipeimportwizard.cpp
        settings/data-wizards/import/pipedimportwizard.cpp
        settings/data-wizards/import/takeoutimportwizard.cpp
        settings/data-wizards/import/shared/chooseentitiespage.cpp
        settings/data-wizards/import/shared/choosesubspage.cpp
        settings/data-wizards/import/shared/choosewatchhistorypage.cpp
        settings/data-wizards/import/shared/importfileselectpage.cpp
        settings/data-wizards/richtableview/richitemdelegate.cpp
        settings/data-wizards/richtableview/richtableview.cpp
        utils/conversion.cpp
        utils/replydata.cpp
        utils/stringutils.cpp
        utils/tubeutils.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            youtubeauth.h
            youtubeplayer.h
            youtubeplugin.h
            settings/channelfiltertable.h
            settings/youtubesettings.h
            settings/youtubesettingsform.h
            settings/data-wizards/basicinfopage.h
            settings/data-wizards/conclusionpage.h
            settings/data-wizards/datawizard.h
            settings/data-wizards/entityselecttablemodel.h
            settings/data-wizards/intropage.h
            settings/data-wizards/import/grayjayimportwizard.h
            settings/data-wizards/import/newpipeimportwizard.h
            settings/data-wizards/import/pipedimportwizard.h
            settings/data-wizards/import/takeoutimportwizard.h
            settings/data-wizards/import/shared/chooseentitiespage.h
            settings/data-wizards/import/shared/choosesubspage.h
            settings/data-wizards/import/shared/choosewatchhistorypage.h
            settings/data-wizards/import/shared/importfileselectpage.h
            settings/data-wizards/richtableview/richitemdelegate.h
            settings/data-wizards/richtableview/richtableview.h
            utils/conversion.h
            utils/replydata.h
            utils/stringutils.h
            utils/tubeutils.h)

set(INNERTUBE_QT_VERSION_MAJOR ${QTTUBE_QT_VERSION_MAJOR} CACHE STRING "Qt version to use, defaults to ${QTTUBE_QT_VERSION_MAJOR}" FORCE)
add_subdirectory(innertube-qt)

set_target_properties(youtube_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
