set(PLUGIN_SOURCES
	xmr-auto-collect.c
)

include_directories(CMAKE_CURRENT_BINARY_DIR)
include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${PLUGIN_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTK3_INCLUDE_DIRS})

add_library(autocollect SHARED ${PLUGIN_SOURCES})
target_link_libraries(autocollect
	xmradio-core
	${LIBPEAS_LIBRARIES}
	${GTK3_LIBRARIES}
)

install(TARGETS autocollect LIBRARY DESTINATION ${plugindir})
install(FILES autocollect.plugin
	DESTINATION ${plugindir})
install(FILES xmr-auto-collect.ui
	DESTINATION ${plugindatadir}/autocollect)
