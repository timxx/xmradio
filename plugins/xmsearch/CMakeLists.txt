set(PLUGIN_SOURCES
	xmsearch.c
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${PLUGIN_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTK3_INCLUDE_DIRS})

add_library(xmsearch SHARED ${PLUGIN_SOURCES})
target_link_libraries(xmsearch
	${LIBPEAS_LIBRARIES}
)

install(TARGETS xmsearch LIBRARY DESTINATION ${plugindir})
install(FILES xmsearch.plugin
	DESTINATION ${plugindir})
