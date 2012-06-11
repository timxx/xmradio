include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${GDKPIXBUF_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GST-PLUGIN_INCLUDE_DIRS})

add_executable(testlibxmrservice testlibxmrservice.c)
target_link_libraries(testlibxmrservice xmrservice ${GOBJECT_LIBRARIES})

add_executable(testxmrskin testxmrskin.c)

target_link_libraries(testxmrskin xmrskin)

add_executable(testxmrplayer testxmrplayer.c)

target_link_libraries(testxmrplayer xmrplayer)
