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

add_executable(testxmrskin testxmrskin.c
	../src/xmrskin.c
	../src/xmrdebug.c
	../src/xmrutil.c
	../src/minizip/ioapi.c
	../src/minizip/unzip.c)

target_link_libraries(testxmrskin ${LIBXML_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GDKPIXBUF_LIBRARIES})

add_executable(testxmrplayer testxmrplayer.c
	../src/xmrplayer.c
	../src/xmrdebug.c
	${CMAKE_BINARY_DIR}/xmrmarshal.c
	)

target_link_libraries(testxmrplayer ${GOBJECT_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GST-PLUGIN_LIBRARIES}
	)
