MACRO(GENERATE_PLUGIN _target)
	set_target_properties(${_target} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${XMR_OUTPUT_PATH}/plugins
	)
ENDMACRO()

MACRO(GENERATE_FILES _files _dir _target)
	foreach(file ${_files})
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${file}"
			COMMAND cmake -E copy
			"${CMAKE_CURRENT_SOURCE_DIR}/${file}"
			"${XMR_OUTPUT_PATH}/${_dir}/${file}"
			DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
		)
		list(APPEND ${_target}_files "${file}")
	endforeach()
	
	add_custom_target(${_target} ALL DEPENDS ${${_target}_files})
ENDMACRO()

MACRO(GENERATE_DIRS _dirs _dir _target)
	foreach(dir ${_dirs})
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
			COMMAND cmake -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/${dir}"
			"${XMR_OUTPUT_PATH}/${_dir}/${dir}"
			DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}"
		)
		list(APPEND ${_target}_files "${dir}")
	endforeach()
	
	add_custom_target(${_target} ALL DEPENDS ${${_target}_files})
ENDMACRO()
