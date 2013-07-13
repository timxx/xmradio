add_subdirectory(notification)
add_subdirectory(indicator)
add_subdirectory(mpris)
add_subdirectory(autocollect)
add_subdirectory(xmsearch)
add_subdirectory(xmrmmkeys)

set(plugins
	notification
	indicator
	mpris
	autocollect
	xmsearch
	xmrmmkeys
)

# Generate plugin data files to
# ${XMR_OUTPUT_PATH}/plugins/

file(GLOB_RECURSE plugin_files RELATIVE
	"${CMAKE_CURRENT_SOURCE_DIR}/" "*.plugin" "*.ui"
)

foreach(file ${plugin_files})
	get_filename_component(filename ${file} NAME)
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${file}"
		COMMAND cmake -E copy
		"${CMAKE_CURRENT_SOURCE_DIR}/${file}"
		"${XMR_OUTPUT_PATH}/plugins/${filename}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
	)
	list(APPEND plugin_data_files "${file}")
endforeach()

add_custom_target(plugin_data ALL DEPENDS ${plugin_data_files})

# Generate plugin modules to
# ${XMR_OUTPUT_PATH}/plugins/
foreach(plugin ${plugins})
	generate_plugin(${plugin})
endforeach()
