set(SKIN_FILES
	pure.skn
	mini.skn
	black.skn
)

install(FILES ${SKIN_FILES} DESTINATION ${skindir})
generate_files("${SKIN_FILES}" "skin" "skin_data")
