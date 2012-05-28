add_subdirectory(lib)
add_subdirectory(plugins)

PKG_CHECK_MODULES(GTK3 REQUIRED "gtk+-3.0")
PKG_CHECK_MODULES(GTHREAD REQUIRED "gthread-2.0")

set(XMRADIO_SOURCES
	main.c
	xmrapp.c
	xmrbutton.c
	xmrplayer.c
	xmrwindow.c
)

include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(xmradio ${XMRADIO_SOURCES})
target_link_libraries(xmradio xmrservice
	${GTK3_LIBRARIES}
	${GTHREAD_LIBRARIES})

install(TARGETS xmradio DESTINATION ${prefix}/bin)
