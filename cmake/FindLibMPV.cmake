# big ups https://github.com/vbalien/cantata/blob/master/cmake/FindLIBMPV.cmake
# only works on linux obviously. will add support for other platforms later

# find includes from client.h
find_path(LIBMPV_INCLUDE_DIR mpv/client.h
    "/usr/include"
    "/usr/include/mpv"
    "/usr/local/include"
    "/usr/local/include/mpv"
)

find_path(LIBMPV_INCLUDE_DIR client.h)

# find library
find_library(LIBMPV_LIBRARY NAMES mpv libmpv PATHS
    "usr/lib"
    "usr/local/lib"
    NO_DEFAULT_PATH
)

find_library(LIBMPV_LIBRARY NAMES mpv libmpv)

if(LIBMPV_INCLUDE_DIR AND LIBMPV_LIBRARY)
    set(LIBMPV_FOUND TRUE)
endif()

if(NOT LIBMPV_FOUND)
    message(FATAL_ERROR "libmpv not found")
endif()

set(LIBMPV_VERSION ${PC_LIBMPV_VERSION})
if(NOT LIBMPV_VERSION)
    file(READ "${LIBMPV_INCLUDE_DIR}/mpv/client.h" _client_h)

    string(REGEX MATCH "MPV_MAKE_VERSION[ ]*\\([ ]*([0-9]+)[ ]*,[ ]*([0-9]+)[ ]*\\)" _dummy "${_client_h}")
    set(_version_major "${CMAKE_MATCH_1}")
    set(_version_minor "${CMAKE_MATCH_2}")

    set(LIBMPV_VERSION "${_version_major}.${_version_minor}")
endif()

message(STATUS "Found libmpv ${LIBMPV_VERSION}")
