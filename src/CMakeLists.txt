add_subdirectory(lib)
add_subdirectory(plugins)

PKG_CHECK_MODULES(GTK3 REQUIRED "gtk+-3.0")

set(XMRADIO_SOURCES
	main.c
)

include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(xmradio ${XMRADIO_SOURCES})
target_link_libraries(xmradio ${GTK3_LIBRARIES})

install(TARGETS xmradio DESTINATION ${prefix}/bin)
