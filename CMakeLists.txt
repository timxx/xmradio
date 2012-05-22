cmake_minimum_required(VERSION 2.6)

project(xmradio)

set(VERSION 0.1.0)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

include(CheckIncludeFiles)
include(FindPkgConfig)

FIND_PACKAGE(Gettext REQUIRED)

set(package	xmradio)
set(prefix	${CMAKE_INSTALL_PREFIX})

set(pkgdatadir	${prefix}/share/${package})
set(localedir	${prefix}/share/locale)

set(CMAKE_C_FLAGS "-Wall -Werror ${CMAKE_C_FLAGS}")

add_subdirectory(cmake)
add_subdirectory(data)
add_subdirectory(doc)
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(test)
