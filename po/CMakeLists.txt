macro(CREATE_TRANSLATIONS _potFile _firstPoFileArg)
	set(_firstPoFile "${_firstPoFileArg}")

	set(_gmoFiles)
	get_filename_component(_potName ${_potFile} NAME)
	string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _potBasename ${_potName})
	get_filename_component(_absPotFile ${_potFile} ABSOLUTE)

	set(_addToAll)
	if(${_firstPoFile} STREQUAL "ALL")
		set(_addToAll "ALL")
		set(_firstPoFile)
	endif()

	foreach(_currentPoFile ${_firstPoFile} ${ARGN})
		get_filename_component(_absFile ${_currentPoFile} ABSOLUTE)
		get_filename_component(_abs_PATH ${_absFile} PATH)
		get_filename_component(_lang ${_absFile} NAME_WE)
		set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
		set(_moFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.mo)

		add_custom_command(
			OUTPUT ${_gmoFile}
			COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none -s ${_absFile} ${_absPotFile}
			COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
			DEPENDS ${_absPotFile} ${_absFile}
		)

		add_custom_command(
			OUTPUT "${_moFile}"
			COMMAND cmake -E copy
			"${_gmoFile}"
			"${XMR_OUTPUT_PATH}/locale/${_lang}/LC_MESSAGES/${package}.mo"
			DEPENDS "${_gmoFile}"
		)

		install(FILES ${_gmoFile} DESTINATION share/locale/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo)
		set(_gmoFiles ${_gmoFiles} ${_gmoFile})
		set(_moFiles ${_moFiles} ${_moFile})
	endforeach()

	add_custom_target(translations ${_addToAll} DEPENDS ${_gmoFiles})
	add_custom_target(generate_translations ALL DEPENDS ${_moFiles})

endmacro()


if (ENABLE_NLS)
	file(GLOB PO_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.po)
	set(POT_FILE xmradio.pot)
	create_translations(${POT_FILE} ALL ${PO_FILES})
endif(ENABLE_NLS)

