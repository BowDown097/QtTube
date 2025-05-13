project(youtube_plugin)

find_package(Qt${QTTUBE_QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(youtube_plugin SHARED)
target_link_libraries(youtube_plugin PUBLIC Qt::Sql)

target_sources(youtube_plugin
    PRIVATE
        youtubeauth.cpp
        youtubeplugin.cpp
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
        utils/tubeutils.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            youtubeauth.h
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
            utils/tubeutils.h)

set(INNERTUBE_QT_VERSION_MAJOR ${QTTUBE_QT_VERSION_MAJOR} CACHE STRING "Qt version to use, defaults to ${QTTUBE_QT_VERSION_MAJOR}" FORCE)
add_subdirectory(innertube-qt)

target_link_libraries(${PROJECT_NAME} PUBLIC innertube-qt QtTube::PluginInterface)

set_target_properties(youtube_plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
