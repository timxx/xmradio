PKG_CHECK_MODULES(APPINDICATOR "appindicator3-0.1")

if (APPINDICATOR_FOUND)
	message(STATUS "Use app indicator")
	set(INDICATOR_SOURCES
		xmr-indicator-plugin.c
		xmr-app-indicator.c
		)
	include_directories(${APPINDICATOR_INCLUDE_DIRS})
	add_definitions("-DHAVE_APP_INDICATOR")
else (APPINDICATOR_FOUND)
	message(STATUS "Use tray icon")
	set(INDICATOR_SOURCES
		xmr-indicator-plugin.c
		xmr-tray-icon.c
		)
endif (APPINDICATOR_FOUND)

include_directories(CMAKE_CURRENT_BINARY_DIR)
include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${PLUGIN_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})

add_library(indicator SHARED ${INDICATOR_SOURCES})
if (APPINDICATOR_FOUND)
	target_link_libraries(indicator
		${LIBPEAS_LIBRARIES}
		${APPINDICATOR_LIBRARIES}
	)
else (APPINDICATOR_FOUND)
	target_link_libraries(indicator
		${LIBPEAS_LIBRARIES}
	)
endif (APPINDICATOR_FOUND)

install(TARGETS indicator LIBRARY DESTINATION ${plugindir})
install(FILES indicator.plugin DESTINATION ${plugindir})
