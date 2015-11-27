set(MPRIS_SOURCES
	xmr-mpris-plugin.c
)

include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${PLUGIN_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTK3_INCLUDE_DIRS})

add_library(mpris SHARED ${MPRIS_SOURCES})
target_link_libraries(mpris
	xmradio-core
	${LIBPEAS_LIBRARIES}
)

install(TARGETS mpris LIBRARY DESTINATION ${plugindir})
install(FILES mpris.plugin DESTINATION ${plugindir})
