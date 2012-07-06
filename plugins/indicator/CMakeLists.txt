PKG_CHECK_MODULES(APPINDICATOR "appindicator3-0.1")

if (APPINDICATOR_FOUND)
	option(ENABLE_APPINDICATOR "Enable app indicator" ON)
else (APPINDICATOR_FOUND)
	option(ENABLE_APPINDICATOR "Enable app indicator" OFF)
endif (APPINDICATOR_FOUND)

if(ENABLE_APPINDICATOR)
	if (NOT APPINDICATOR_FOUND)
		message(FATAL_ERROR "package 'appindicator3-0.1' not found")
	endif(NOT APPINDICATOR_FOUND)

	message(STATUS "Use app indicator")
	set(INDICATOR_SOURCES
		xmr-indicator-plugin.c
		xmr-app-indicator.c
		)
	include_directories(${APPINDICATOR_INCLUDE_DIRS})
	add_definitions("-DHAVE_APP_INDICATOR")
else (ENABLE_APPINDICATOR)
	message(STATUS "Use tray icon")
	set(INDICATOR_SOURCES
		xmr-indicator-plugin.c
		xmr-tray-icon.c
		)
endif (ENABLE_APPINDICATOR)

include_directories(CMAKE_CURRENT_BINARY_DIR)
include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${PLUGIN_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})

add_library(indicator SHARED ${INDICATOR_SOURCES})
if (ENABLE_APPINDICATOR)
	target_link_libraries(indicator
		${LIBPEAS_LIBRARIES}
		${APPINDICATOR_LIBRARIES}
	)
else (ENABLE_APPINDICATOR)
	target_link_libraries(indicator
		${LIBPEAS_LIBRARIES}
	)
endif (ENABLE_APPINDICATOR)

install(TARGETS indicator LIBRARY DESTINATION ${plugindir})
install(FILES indicator.plugin DESTINATION ${plugindir})
