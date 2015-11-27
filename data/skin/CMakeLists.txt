set(skin_DRIS
	black
	doubanfm
	mini
	pure
)

install(DIRECTORY ${skin_DRIS} DESTINATION ${skindir})
generate_dirs("${skin_DRIS}" "skin" "skin_data")
