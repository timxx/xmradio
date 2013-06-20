find_program(UPDATE_ICON gtk-update-icon-cache)
if (UPDATE_ICON)
	execute_process(COMMAND
		${UPDATE_ICON} -f -t "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor"
	)
endif()
