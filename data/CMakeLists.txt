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

MACRO(TRANSLATE_XML infile outfile)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${outfile}
		COMMAND ${INTLTOOL_MERGE} -x -u
		${i18ndir}
		${infile} ${outfile}
		DEPENDS ${infile}
	)
ENDMACRO(TRANSLATE_XML)

add_custom_target(schemasfile ALL DEPENDS ${gsettings_SCHEMAS})
# translate_xml(${CMAKE_CURRENT_SOURCE_DIR}/com.timxx.xmradio.gschema.xml.in
#	${CMAKE_CURRENT_BINARY_DIR}/com.timxx.xmradio.gschema.xml)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.timxx.xmradio.gschema.xml
	DESTINATION ${prefix}/share/glib-2.0/schemas/)
install(SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/compile-schemas.cmake)

install(DIRECTORY ui DESTINATION ${pkgdatadir}
	PATTERN .* EXCLUDE
	PATTERN *~ EXCLUDE)
