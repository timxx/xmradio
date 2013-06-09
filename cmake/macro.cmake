MACRO(GENERATE_PLUGIN _target)
	set_target_properties(${_target} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${XMR_OUTPUT_PATH}/plugins
	)
ENDMACRO()
