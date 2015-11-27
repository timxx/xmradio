include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${GTK3_INCLUDE_DIRS})

add_executable(testlibxmrservice testlibxmrservice.c)
target_link_libraries(testlibxmrservice xmrservice ${GOBJECT_LIBRARIES})

add_executable(testxmrskin testxmrskin.c)

target_link_libraries(testxmrskin xmradio-core)

add_executable(testxmrplayer testxmrplayer.c)

target_link_libraries(testxmrplayer xmradio-core ${GOBJECT_LIBRARIES})
