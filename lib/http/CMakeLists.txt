cmake_minimum_required(VERSION 3.16)
project(http VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC, AUTORCC, and AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
include(GNUInstallDirs)

# Compiler flags
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network)

add_library(http
    src/cachedhttp.cpp src/cachedhttp.h
    src/http.cpp src/http.h
    src/httpreply.cpp src/httpreply.h
    src/httprequest.h
    src/localcache.cpp src/localcache.h
    src/networkhttpreply.cpp src/networkhttpreply.h
    src/throttledhttp.cpp src/throttledhttp.h
)

target_include_directories(http PUBLIC src)
target_compile_definitions(http PUBLIC HTTP)
# target_compile_definitions(http PUBLIC HTTP_DEBUG) # helpful logs for debugging library and requests
target_link_libraries(http PUBLIC Qt::Core Qt::Gui Qt::Network)

install(TARGETS http
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
