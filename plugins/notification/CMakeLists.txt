PKG_CHECK_MODULES(LIBNOTIFY REQUIRED "libnotify")

set(PLUGIN_SOURCES
	xmr-notification-plugin.c
)

include_directories(CMAKE_CURRENT_BINARY_DIR)
include_directories(${LIBPEAS_INCLUDE_DIRS})
include_directories(${PLUGIN_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${LIBNOTIFY_INCLUDE_DIRS})

add_library(notification SHARED ${PLUGIN_SOURCES})
target_link_libraries(notification
	${LIBPEAS_LIBRARIES}
	${LIBNOTIFY_LIBRARIES}
)

install(TARGETS notification LIBRARY DESTINATION ${plugindir})
install(FILES notification.plugin
	DESTINATION ${plugindir})
