cmake_minimum_required(VERSION 3.23)
project(QtTube
    VERSION 1.3
    DESCRIPTION "A minimalist, cross-platform desktop app for streaming media."
    HOMEPAGE_URL "https://github.com/BowDown097/QtTube"
    LANGUAGES C CXX
)

# CMake options
## Needed for includes in ui files
set(CMAKE_INCLUDE_CURRENT_DIR ON)
## Standard version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
## Output executables and libs in the main build folder, not in subfolders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
if(UNIX)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endif()

# Names
## App
set(QTTUBE_APP_NAME "${PROJECT_NAME}")
set(QTTUBE_APP_DESC "${PROJECT_DESCRIPTION}")
string(TOLOWER "${QTTUBE_APP_NAME}" QTTUBE_BINARY_NAME)
## Version name
set(QTTUBE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(QTTUBE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(QTTUBE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(QTTUBE_VERSION_TWEAK "${PROJECT_VERSION_TWEAK}")
set(QTTUBE_VERSION_NAME "${PROJECT_VERSION}")
if(QTTUBE_VERSION_TWEAK)
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.${QTTUBE_VERSION_PATCH}.${QTTUBE_VERSION_TWEAK}")
elseif(QTTUBE_VERSION_PATCH)
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.${QTTUBE_VERSION_PATCH}.0")
else()
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.0.0")
endif()

# macOS bundle settings
if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "${QTTUBE_APP_NAME}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${QTTUBE_VERSION_NAME}")
    set(MACOSX_BUNDLE_INFO_STRING "${QTTUBE_APP_NAME}: ${QTTUBE_APP_DESC}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${QTTUBE_VERSION_NAME}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${QTTUBE_VERSION_NAME}")
endif()

# LTO
option(QTTUBE_ENABLE_LTO "Use Link Time Optimization (release builds only)" OFF)
if(QTTUBE_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT lto_output)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        message(STATUS "LTO/IPO is enabled for release builds")
    else()
        message(STATUS "LTO/IPO requested but it is not supported by the compiler: ${lto_output}")
    endif()
endif()

# Options
option(QJS_BUILD_LIBC "Build standard library modules as part of the library" ON)
option(QTTUBE_ENABLE_WEB_COMPONENTS "Enable usage of WebEngine-based components" ON)

# Qt project setup
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

# Subprojects
add_subdirectory(external/plugin_interface)
add_subdirectory(external/quickjs++)
add_subdirectory(program_info)
add_subdirectory(application)
