add_subdirectory(lib)
add_subdirectory(plugins)

PKG_CHECK_MODULES(GTK3 REQUIRED "gtk+-3.0")
PKG_CHECK_MODULES(GTHREAD REQUIRED "gthread-2.0")
PKG_CHECK_MODULES(ZLIB REQUIRED "zlib")
PKG_CHECK_MODULES(GDKPIXBUF REQUIRED "gdk-pixbuf-2.0")
PKG_CHECK_MODULES(GSTREAMER REQUIRED "gstreamer-0.10")
PKG_CHECK_MODULES(GST-PLUGIN REQUIRED "gstreamer-interfaces-0.10")

execute_process(COMMAND ${GLIB_GENMARSHAL} --body
	--prefix=xmr_marshal ${CMAKE_CURRENT_SOURCE_DIR}/xmrmarshal.list
	OUTPUT_FILE xmrmarshal.c
)

execute_process(COMMAND ${GLIB_GENMARSHAL} --header
	--prefix=xmr_marshal ${CMAKE_CURRENT_SOURCE_DIR}/xmrmarshal.list
	OUTPUT_FILE xmrmarshal.h
)

set(XMRADIO_SOURCES
	main.c
	xmrapp.c
	xmrbutton.c
	xmrdebug.c
	xmrplayer.c
	xmrradio.c
	xmrradiochooser.c
	xmrsettings.c
	xmrskin.c
	xmrutil.c
	xmrwindow.c
	minizip/ioapi.c
	minizip/unzip.c
	${CMAKE_BINARY_DIR}/xmrmarshal.c
)

include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${GDKPIXBUF_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})

add_executable(xmradio ${XMRADIO_SOURCES})
target_link_libraries(xmradio xmrservice
	${GTK3_LIBRARIES}
	${GTHREAD_LIBRARIES}
	${ZLIB_LIBRARIES}
	${GDKPIXBUF_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GST-PLUGIN_LIBRARIES}
	)

install(TARGETS xmradio DESTINATION ${prefix}/bin)
