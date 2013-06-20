add_subdirectory(skin)
add_subdirectory(icons)

set(DESKTOP_FILES
	xmradio.desktop
)

find_program(INTLTOOL_EXTRACT intltool-extract)
find_program(INTLTOOL_UPDATE intltool-update)
find_program(INTLTOOL_MERGE intltool-merge)

MACRO(INTLTOOL_MERGE_TRANSLATION infile outfile)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${outfile}
		COMMAND LC_ALL=C ${INTLTOOL_MERGE} -d -u ${i18ndir} ${infile} ${outfile}
		DEPENDS ${infile}
	)
ENDMACRO(INTLTOOL_MERGE_TRANSLATION)

intltool_merge_translation(${CMAKE_CURRENT_SOURCE_DIR}/xmradio.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/xmradio.desktop )

add_custom_target(desktopfile ALL DEPENDS ${DESKTOP_FILES})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xmradio.desktop DESTINATION ${prefix}/share/applications/)

set(gsettings_SCHEMAS
	com.timxx.xmradio.gschema.xml
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.timxx.xmradio.gschema.xml
	DESTINATION ${prefix}/share/glib-2.0/schemas/)
install(SCRIPT ${XMR_CMAKE_PATH}/compile-schemas.cmake)

install(DIRECTORY ui DESTINATION ${pkgdatadir}
	PATTERN .* EXCLUDE
	PATTERN *~ EXCLUDE)

# Generate *.ui files to
# ${XMR_OUTPUT_PATH}/ui
generate_dirs("ui" "" "ui_data")

# Generate schema files
generate_files("${gsettings_SCHEMAS}" "glib-2.0/schemas" "schema_data")

find_program(COMPILE_SCHEMA glib-compile-schemas)
if (COMPILE_SCHEMA)
	add_custom_command(TARGET schema_data POST_BUILD
		COMMAND ${COMPILE_SCHEMA} "${XMR_OUTPUT_PATH}/glib-2.0/schemas"
	)
endif()
