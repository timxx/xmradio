add_subdirectory(lib)

PKG_CHECK_MODULES(GTK3 REQUIRED "gtk+-3.0")
PKG_CHECK_MODULES(GTHREAD REQUIRED "gthread-2.0")
PKG_CHECK_MODULES(LIBVLC REQUIRED "libvlc >= 2.0")
PKG_CHECK_MODULES(SQLITE3 REQUIRED "sqlite3")
PKG_CHECK_MODULES(DBUS REQUIRED "dbus-glib-1")
PKG_CHECK_MODULES(LIBPEAS REQUIRED "libpeas-1.0")
PKG_CHECK_MODULES(LIBPEAS-GTK REQUIRED "libpeas-gtk-1.0")

execute_process(COMMAND ${GLIB_GENMARSHAL} --body
	--prefix=xmr_marshal ${CMAKE_CURRENT_SOURCE_DIR}/xmrmarshal.list
	OUTPUT_FILE xmrmarshal.c
)

execute_process(COMMAND ${GLIB_GENMARSHAL} --header
	--prefix=xmr_marshal ${CMAKE_CURRENT_SOURCE_DIR}/xmrmarshal.list
	OUTPUT_FILE xmrmarshal.h
)

set(XMRADIO_HEADERS
	xmrapp.h
	xmrartist.h
	xmrbutton.h
	xmrchooser.h
	xmrdb.h
	xmrdebug.h
	xmrdownloader.h
	xmrlabel.h
	xmrlist.h
	xmrplayer.h
	xmrpluginengine.h
	xmrradio.h
	xmrsearchbox.h
	xmrsettings.h
	xmrskin.h
	xmrutil.h
	xmrvolumebutton.h
	xmrwindow.h
)

set(XMRADIO_SOURCES
	main.c
)

set(XMRADIO_CORE_SOURCES
	xmrapp.c
	xmrartist.c
	xmrbutton.c
	xmrchooser.c
	xmrdb.c
	xmrdebug.c
	xmrdownloader.c
	xmrlabel.c
	xmrlist.c
	xmrplayer.c
	xmrpluginengine.c
	xmrradio.c
	xmrsearchbox.c
	xmrsettings.c
	xmrskin.c
	xmrutil.c
	xmrvolumebutton.c
	xmrwindow.c
	${CMAKE_BINARY_DIR}/xmrmarshal.c
)

include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})
include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${LIBPEAS-GTK_INCLUDE_DIRS})

add_library(xmradio-core SHARED ${XMRADIO_CORE_SOURCES})
target_link_libraries(xmradio-core
	xmrservice
	${GTK3_LIBRARIES}
	${SQLITE3_LIBRARIES}
	${DBUS_LIBRARIES}
	${LIBPEAS-GTK_LIBRARIES}
	${LIBXML_LIBRARIES}
	${LIBVLC_LIBRARIES}
)

add_executable(xmradio ${XMRADIO_SOURCES})
target_link_libraries(xmradio xmradio-core)

configure_file(xmradio.pc.in ${CMAKE_CURRENT_BINARY_DIR}/xmradio.pc)

install(TARGETS xmradio-core LIBRARY DESTINATION ${libdir})
install(TARGETS xmradio DESTINATION ${prefix}/bin)
install(FILES ${XMRADIO_HEADERS} DESTINATION ${includedir}/xmradio)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xmradio.pc DESTINATION "${libdir}/pkgconfig")
