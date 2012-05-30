include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${GDKPIXBUF_INCLUDE_DIRS})

add_executable(testlibxmrservice testlibxmrservice.c)
target_link_libraries(testlibxmrservice xmrservice ${GOBJECT_LIBRARIES})

add_executable(testxmrskin testxmrskin.c
	../src/xmrskin.c
	../src/xmrdebug.c
	../src/minizip/ioapi.c
	../src/minizip/unzip.c)

target_link_libraries(testxmrskin ${LIBXML_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GDKPIXBUF_LIBRARIES})
