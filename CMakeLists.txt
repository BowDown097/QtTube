cmake_minimum_required(VERSION 3.23)
project(QtTube
    VERSION 1.1
    DESCRIPTION "A minimalist, cross-platform desktop app for streaming media."
    HOMEPAGE_URL "https://github.com/BowDown097/QtTube"
    LANGUAGES CXX C)

# CMake options
## Needed for includes in ui files
set(CMAKE_INCLUDE_CURRENT_DIR ON)
## MOC, RCC, and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
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
set(QTTUBE_APP_DESC "${CMAKE_PROJECT_DESCRIPTION}")
set(QTTUBE_APP_NAME "${CMAKE_PROJECT_NAME}")
string(TOLOWER "${QTTUBE_APP_NAME}" QTTUBE_BINARY_NAME)
## Version name
set(QTTUBE_VERSION_MAJOR "${CMAKE_PROJECT_VERSION_MAJOR}")
set(QTTUBE_VERSION_MINOR "${CMAKE_PROJECT_VERSION_MINOR}")
set(QTTUBE_VERSION_NAME "${CMAKE_PROJECT_VERSION}")
if(CMAKE_PROJECT_VERSION_TWEAK)
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.\
                              ${CMAKE_PROJECT_VERSION_PATCH}.${CMAKE_PROJECT_VERSION_TWEAK}")
elseif(CMAKE_PROJECT_VERSION_PATCH)
    set(QTTUBE_VERSION_NAME4 "${QTTUBE_VERSION_MAJOR}.${QTTUBE_VERSION_MINOR}.\
                              ${CMAKE_PROJECT_VERSION_PATCH}.0")
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
option(ENABLE_LTO "Enable Link Time Optimization" OFF)
if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

    if(ipo_supported AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"))
        message(STATUS "IPO / LTO enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    elseif(ipo_supported)
        message(STATUS "Not enabling IPO / LTO on debug builds")
    else()
        message(STATUS "IPO / LTO not supported: <${ipo_error}>")
    endif()
endif()

# Components
option(ENABLE_WEB_COMPONENTS "" ON)
add_subdirectory(program_info)
add_subdirectory(plugin_interface)
add_subdirectory(application)
