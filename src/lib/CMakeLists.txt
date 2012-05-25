PKG_CHECK_MODULES(GLIB REQUIRED "glib-2.0")
PKG_CHECK_MODULES(CURL REQUIRED "libcurl")

set(LIB_SOURCES
	xmrservice.c
)

set(LIB_HEADERS
	xmrservice.h
	songinfo.h
)

set(LIB_VERSION 0.1)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(xmrservice SHARED ${LIB_SOURCES})
set_target_properties(xmrservice
	PROPERTIES VERSION ${LIB_VERSION}
	SOVERSION 0
	COMPILE_FLAGS "-fvisibility=hidden"
)

target_link_libraries(xmrservice ${GLIB_LIBRARIES}
	${CURL_LIBRARIES}
)

configure_file(xmrservice.pc.in ${CMAKE_CURRENT_BINARY_DIR}/xmrservice.pc)

install(TARGETS xmrservice LIBRARY DESTINATION ${libdir})
install(FILES ${LIB_HEADERS} DESTINATION "${includedir}/xmradio")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xmrservice.pc DESTINATION "${libdir}/pkconfig")
