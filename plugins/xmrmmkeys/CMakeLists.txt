set(XMRMMKEYS_SOURCES
	xmrmmkeys.c
)

include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${PLUGIN_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})

add_library(xmrmmkeys SHARED ${XMRMMKEYS_SOURCES})
target_link_libraries(xmrmmkeys
	${LIBPEAS_LIBRARIES}
)

install(TARGETS xmrmmkeys LIBRARY DESTINATION ${plugindir})
install(FILES xmrmmkeys.plugin DESTINATION ${plugindir})

